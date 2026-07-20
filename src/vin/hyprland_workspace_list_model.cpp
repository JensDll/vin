#include "vin/hyprland_command.hpp"
#include "vin/hyprland_workspace_list_model.hpp"
#include "vin/main_context.hpp"

#include <fmt/format.h>
#include <fmt/std.h>
#include <peel/class.h>
#include <peel/Gio/Initable.h>
#include <peel/Gio/ListModel.h>
#include <spdlog/spdlog.h>

#include <charconv>
#include <iterator>
#include <utility>

using namespace peel;
using namespace vin;

PEEL_CLASS_IMPL(HyprlandWorkspaceListModel, "VinHyprlandWorkspaceListModel", Object)

void HyprlandWorkspaceListModel::Class::init()
{
  override_vfunc_dispose<HyprlandWorkspaceListModel>();
}

void HyprlandWorkspaceListModel::init_type(const Type type)
{
  PEEL_IMPLEMENT_INTERFACE(type, Gio::Initable);
  PEEL_IMPLEMENT_INTERFACE(type, Gio::ListModel);
}

void HyprlandWorkspaceListModel::init_interface(Gio::Initable::Iface* const iface)
{
  iface->override_vfunc_init<HyprlandWorkspaceListModel>();
}

void HyprlandWorkspaceListModel::init_interface(Gio::ListModel::Iface* const iface)
{
  iface->override_vfunc_get_item<HyprlandWorkspaceListModel>();
  iface->override_vfunc_get_item_type<HyprlandWorkspaceListModel>();
  iface->override_vfunc_get_n_items<HyprlandWorkspaceListModel>();
}

void HyprlandWorkspaceListModel::vfunc_dispose()
{
  spdlog::info("[VinHyprlandWorkspaceListModel] dispose");
  m_event_listener = nullptr;
  parent_vfunc_dispose<HyprlandWorkspaceListModel>();
}

bool HyprlandWorkspaceListModel::vfunc_init(Gio::Cancellable* const cancellable, UniquePtr<GLib::Error>* const error)
{
  m_event_listener =
    HyprlandEventListener::create({ .main_context = m_main_context, .worker_context = m_worker_context });

  if (!m_event_listener->cast<Gio::Initable>()->init(cancellable, error)) {
    return false;
  }

  m_event_listener->connect_createworkspacev2(this, &HyprlandWorkspaceListModel::on_new_workspace);
  m_event_listener->connect_destroyworkspacev2(this, &HyprlandWorkspaceListModel::on_remove_workspace);
  m_event_listener->connect_moveworkspacev2(this, &HyprlandWorkspaceListModel::on_remove_workspace);
  m_event_listener->connect_workspacev2(this, &HyprlandWorkspaceListModel::on_workspace_change);
  m_event_listener->connect_activespecialv2(this, &HyprlandWorkspaceListModel::on_active_special);

  auto command{ HyprlandCommand::create(error) };
  if (!command) {
    return false;
  }
  auto response{ command->send("j/workspaces", error) };
  if (!response) {
    return false;
  }

  const auto workspaces_json{ nlohmann::json::parse(response.value()) };

  command = HyprlandCommand::create(error);
  if (!command) {
    return false;
  }
  response = command->send("j/activeworkspace", error);
  if (!response) {
    return false;
  }

  const auto active_workspace_json{ nlohmann::json::parse(response.value()) };
  active_workspace_json.at("id").get_to(m_active_id);

  new (&m_items) decltype(m_items);

  for (const auto& [key, value] : workspaces_json.items()) {
    const auto workspace{ HyprlandWorkspace::create(value) };
    if (workspace->get_id() == m_active_id) {
      workspace->set_is_active(true);
    }
    m_num_special += static_cast<decltype(m_num_special)>(workspace->get_is_special());
    m_items.try_emplace(workspace->get_id(), workspace);
  }

  if (m_num_special > 0) {
    notify(prop_is_any_special());
  }

  items_changed(0, 0, m_items.size());

  return true;
}

namespace {
int parse_workspace_id(const char* const data)
{
  int result{};
  std::from_chars(data, std::strchr(data, ','), result);
  return result;
}
} // namespace

void HyprlandWorkspaceListModel::on_workspace_change([[maybe_unused]] HyprlandEventListener* const event_listener,
  const char* const data)
{
  g_assert(GLib::MainContext::default_()->is_owner());

  const auto id{ parse_workspace_id(data) };

  if (const auto it{ m_items.find(m_active_id) }; it != m_items.end()) {
    it->second->set_is_active(false);
  }

  if (const auto it{ m_items.find(m_active_id = id) }; it != m_items.end()) {
    it->second->set_is_active(true);
  }
}

void HyprlandWorkspaceListModel::on_new_workspace([[maybe_unused]] HyprlandEventListener* const event_listener,
  const char* const data)
{
  g_assert(GLib::MainContext::default_()->is_owner());

  const auto id{ parse_workspace_id(data) };

  UniquePtr<GLib::Error> error;
  auto command{ HyprlandCommand::create(&error) };
  if (!command) {
    spdlog::error(error->message);
    return;
  }
  auto response{ command->send("j/workspaces", &error) };
  if (!response) {
    spdlog::error(error->message);
    return;
  }

  const auto workspaces_json{ nlohmann::json::parse(response.value()) };

  for (const auto& [key, value] : workspaces_json.items()) {
    if (id != value.at("id").get<int>()) {
      continue;
    }
    const auto workspace{ HyprlandWorkspace::create(value) };
    if (workspace->get_is_special() && ++m_num_special == 1) {
      notify(prop_is_any_special());
    }
    const auto& [it, success]{ m_items.try_emplace(id, workspace) };
    g_assert(success);
    items_changed(std::distance(m_items.begin(), it), 0, 1);
    break;
  }
}

void HyprlandWorkspaceListModel::on_remove_workspace([[maybe_unused]] HyprlandEventListener* const event_listener,
  const char* const data)
{
  g_assert(GLib::MainContext::default_()->is_owner());

  const auto id{ parse_workspace_id(data) };

  const auto it{ m_items.find(id) };

  if (it == m_items.end()) {
    spdlog::error("tried to remove workspace with id {}, but the item is not in the container", id);
    return;
  }

  const auto position{ std::distance(m_items.begin(), it) };

  if (it->second->get_is_special() && --m_num_special == 0) {
    notify(prop_is_any_special());
  }

  m_items.erase(it);

  items_changed(position, 1, 0);
}

void HyprlandWorkspaceListModel::on_active_special([[maybe_unused]] HyprlandEventListener* const event_listener,
  const char* const data)
{
  g_assert(GLib::MainContext::default_()->is_owner());

  if (const auto it{ m_items.find(m_active_special_id) }; it != m_items.end()) {
    it->second->set_is_active(false);
  }

  if (data[0] == ',') {
    return;
  }

  const auto id{ parse_workspace_id(data) };

  if (const auto it{ m_items.find(m_active_special_id = id) }; it != m_items.end()) {
    it->second->set_is_active(true);
  }
}

bool HyprlandWorkspaceListModel::get_is_any_special() const
{
  return m_num_special > 0;
}

VIN_IMPLEMENT_MAIN_CONTEXT_PROPERTY(HyprlandWorkspaceListModel)

void HyprlandWorkspaceListModel::define_properties(auto& visitor)
{
  VIN_DEFINE_MAIN_CONTEXT_PROPERTY(HyprlandWorkspaceListModel)
  visitor.prop(prop_is_any_special(), false).get(&HyprlandWorkspaceListModel::get_is_any_special);
}
