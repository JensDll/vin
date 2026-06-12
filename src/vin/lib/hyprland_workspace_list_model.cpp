#include "vin/lib/hyprland_command.hpp"
#include "vin/lib/hyprland_workspace_list_model.hpp"

#include <fmt/std.h>
#include <peel/class.h>
#include <peel/Gio/Initable.h>
#include <peel/Gio/ListModel.h>
#include <spdlog/spdlog.h>

#include <charconv>
#include <iterator>
#include <utility>

using namespace peel;
using namespace vin::lib;

PEEL_CLASS_IMPL(HyprlandWorkspaceListModel, "VinHyprlandWorkspaceListModel", Object)

HyprlandWorkspaceListModel::SignalNewSpecial HyprlandWorkspaceListModel::s_signal_new_special;
HyprlandWorkspaceListModel::SignalRemoveSpecial HyprlandWorkspaceListModel::s_signal_remove_special;

void HyprlandWorkspaceListModel::Class::init()
{
  s_signal_new_special = SignalNewSpecial::create("new-special");
  s_signal_remove_special = SignalRemoveSpecial::create("remove-special");
  override_vfunc_dispose<HyprlandWorkspaceListModel>();
}

void HyprlandWorkspaceListModel::vfunc_dispose()
{
  spdlog::info("dispose workspace list model {}", fmt::ptr(this));
  m_event_listener = nullptr;
  parent_vfunc_dispose<HyprlandWorkspaceListModel>();
}

bool HyprlandWorkspaceListModel::vfunc_init(Gio::Cancellable* const cancellable, UniquePtr<GLib::Error>* const error)
{
  spdlog::info("new workspace list model");

  m_event_listener = HyprlandEventListener::create(cancellable, error, m_main_context, m_worker_context);

  if (!m_event_listener) {
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
    if (workspace->get_is_special()) {
      s_signal_new_special.emit(this, ++m_num_special);
    }
    spdlog::info("new workspace with id {} {}", id, fmt::ptr(this));
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

  if (it->second->get_is_special()) {
    s_signal_remove_special.emit(this, --m_num_special);
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
