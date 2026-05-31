#include "vin/lib/hyprland_command.hpp"
#include "vin/lib/hyprland_workspace_list_model.hpp"

#include <charconv>
#include <iterator>
#include <utility>

using namespace peel;
using namespace vin::lib;

PEEL_CLASS_IMPL(HyprlandWorkspaceListModel, "VinHyprlandWorkspaceListModel", Object)

HyprlandWorkspaceListModel::SignalNoSpecial HyprlandWorkspaceListModel::s_signal_no_special =
  HyprlandWorkspaceListModel::SignalNoSpecial::create("no_special");

void HyprlandWorkspaceListModel::init([[maybe_unused]] Class* const cls)
{
  const auto event_listener{ HyprlandEventListener::singleton() };

  event_listener->connect_createworkspacev2(this, &HyprlandWorkspaceListModel::on_new_workspace);
  event_listener->connect_destroyworkspacev2(this, &HyprlandWorkspaceListModel::on_remove_workspace);
  event_listener->connect_moveworkspacev2(this, &HyprlandWorkspaceListModel::on_remove_workspace);
  event_listener->connect_workspacev2(this, &HyprlandWorkspaceListModel::on_workspace_change);
  event_listener->connect_activespecialv2(this, &HyprlandWorkspaceListModel::on_active_special);

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

  command = HyprlandCommand::create(&error);
  if (!command) {
    spdlog::error(error->message);
    return;
  }
  response = command->send("j/activeworkspace", &error);
  if (!response) {
    spdlog::error(error->message);
    return;
  }

  const auto active_workspace_json{ nlohmann::json::parse(response.value()) };
  active_workspace_json.at("id").get_to(m_active_id);

  new (&m_items) decltype(m_items);

  for (const auto& [key, value] : workspaces_json.items()) {
    const auto workspace{ HyprlandWorkspace::create(value) };
    if (workspace->id() == m_active_id) {
      workspace->set_is_active(true);
    }
    m_num_special += static_cast<decltype(m_num_special)>(workspace->is_special());
    m_items.try_emplace(workspace->id(), workspace);
  }
}

void HyprlandWorkspaceListModel::Class::init() {}

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
    m_num_special += static_cast<decltype(m_num_special)>(workspace->is_special());
    const auto& [it, success]{ m_items.try_emplace(workspace->id(), workspace) };
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

  m_num_special -= static_cast<decltype(m_num_special)>(it->second->is_special());

  m_items.erase(it);

  if (m_num_special == 0) {
    s_signal_no_special.emit(this);
  }

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
