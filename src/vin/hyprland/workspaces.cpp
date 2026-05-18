#include "vin/hyprland/command.hpp"
#include "vin/hyprland/event_listener.hpp"
#include "vin/hyprland/event_parser.hpp"
#include "vin/hyprland/workspace_button.hpp"
#include "vin/hyprland/workspaces.hpp"

#include <peel/GObject/Object.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <peel/class.h>
#include <peel/FloatPtr.h>
#include <peel/Gio/ListStore.h>
#include <peel/GLib/Error.h>
#include <peel/GLib/functions.h>
#include <peel/GLib/MainContext.h>
#include <peel/Gtk/Box.h>
#include <peel/Gtk/Button.h>
#include <peel/Gtk/ClosureExpression.h>
#include <peel/Gtk/DropDown.h>
#include <peel/Gtk/Label.h>
#include <peel/Gtk/PropertyExpression.h>
#include <peel/Gtk/Widget.h>
#include <peel/signal.h>
#include <peel/UniquePtr.h>
#include <spdlog/spdlog.h>

#include <array>
#include <iterator>
#include <tuple>
#include <utility>

using namespace peel;
using namespace vin::hyprland;

PEEL_CLASS_IMPL(Workspaces, "VinHyprlandWorkspaces", Gtk::Box)

auto Workspaces::insert_workspace(const nlohmann::json& json) -> decltype(m_workspaces)::const_iterator
{
  auto button{ WorkspaceButton::create(json) };

  const auto [it, success]{ m_workspaces.try_emplace(button->get_id(),
    button,
    button->connect_clicked(
      [request = fmt::format("dispatch hl.dsp.focus({{workspace='{}'}})", button->get_id())](Gtk::Button*) {
        spdlog::info("button clicked ... {}", request);
        UniquePtr<GLib::Error> error;
        const auto command{ Command::create(&error) };
        if (!command) {
          spdlog::error(error->message);
          return;
        }
        command->send_and_forget(request, nullptr);
      })) };

  if (button->is_special()) {
    m_dropdown_items->append(button);
  } else {
    auto label{ Gtk::Label::create(it->second.button->get_name()) };
    button->set_child(std::move(label));
    if (it == m_workspaces.begin()) {
      insert_child_after(std::move(button), m_dropdown);
    } else {
      insert_child_after(std::move(button), std::prev(it)->second.button);
    }
  }

  return it;
}

void Workspaces::init(Class* const /*cls*/)
{
  const auto event_listener{ EventListener::create() };

  event_listener->connect_createworkspacev2(this, &Workspaces::on_new_workspace);
  event_listener->connect_destroyworkspacev2(this, &Workspaces::on_remove_workspace);
  event_listener->connect_moveworkspacev2(this, &Workspaces::on_remove_workspace);
  event_listener->connect_workspacev2(this, &Workspaces::on_workspace_change);
  event_listener->connect_activespecialv2(this, &Workspaces::on_active_special);

  UniquePtr<GLib::Error> error;
  auto command{ Command::create(&error) };
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

  command = Command::create(&error);
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

  new (&m_workspaces) decltype(m_workspaces);

  auto store{ Gio::ListStore::create(Type::of<WorkspaceButton>()) };

  m_dropdown_items = store;

  auto name_expression{ Gtk::PropertyExpression::create(
    Type::of<WorkspaceButton>(), nullptr, WorkspaceButton::prop_special_name().get_name()) };

  auto dropdown{ Gtk::DropDown::create(std::move(store), std::move(name_expression)) };

  dropdown->set_show_arrow(false);

  dropdown->connect_notify(
    peel::Gtk::DropDown::prop_selected_item(),
    [](peel::GObject::Object* const obj, peel::GObject::ParamSpec* const) {
      const auto request{ fmt::format("dispatch hl.dsp.focus({{workspace='{}'}})",
        obj->cast<Gtk::DropDown>()->get_selected_item()->cast<WorkspaceButton>()->get_name()) };
      spdlog::info("dropdown selected ... {}", request);
      UniquePtr<GLib::Error> error;
      const auto command{ Command::create(&error) };
      if (!command) {
        spdlog::error(error->message);
        return;
      }
      command->send_and_forget(request, nullptr);
    },
    true);

  m_dropdown = dropdown;

  append(std::move(dropdown));

  for (const auto& [key, value] : workspaces_json.items()) {
    insert_workspace(value);
  }
}

void Workspaces::Class::init()
{
  override_vfunc_dispose<Workspaces>();
}

void Workspaces::vfunc_dispose()
{
  for (auto& [key, value] : m_workspaces) {
    value.click_connection.disconnect();
  }
  parent_vfunc_dispose<Workspaces>();
}

void Workspaces::on_workspace_change(EventListener* /*event_listener*/, const char* const data)
{
  g_assert(GLib::MainContext::default_()->is_owner());
  const auto id{ parse_workspace_id(data) };
  if (const auto it{ m_workspaces.find(m_active_id) }; it != m_workspaces.end()) {
    it->second.button->remove_css_class("active");
  }
  if (const auto it{ m_workspaces.find(m_active_id = id) }; it != m_workspaces.end()) {
    it->second.button->add_css_class("active");
  }
}

void Workspaces::on_new_workspace(EventListener* /*event_listener*/, const char* const data)
{
  g_assert(GLib::MainContext::default_()->is_owner());

  const auto id{ parse_workspace_id(data) };

  UniquePtr<GLib::Error> error;
  const auto command{ Command::create(&error) };
  if (!command) {
    spdlog::error(error->message);
    return;
  }
  const auto response{ command->send("j/workspaces", &error) };
  if (!response) {
    spdlog::error(error->message);
    return;
  }
  const auto json{ nlohmann::json::parse(response.value()) };

  for (const auto& [key, value] : json.items()) {
    if (id == value.at("id").get<int>()) {
      insert_workspace(value);
      break;
    }
  }
}

void Workspaces::on_remove_workspace(EventListener* /*event_listener*/, const char* const data)
{
  g_assert(GLib::MainContext::default_()->is_owner());
  spdlog::info("remove workspace {}", data);
  const auto id{ parse_workspace_id(data) };
  const auto handle{ m_workspaces.extract(id) };
  if (!handle.empty()) {
    handle.mapped().click_connection.disconnect();
    if (handle.mapped().button->is_special()) {
      unsigned int pos{};
      if (m_dropdown_items->find(handle.mapped().button, &pos)) {
        m_dropdown_items->remove(pos);
      }
    } else {
      remove(handle.mapped().button);
    }
  } else {
    spdlog::error("Failed to remove workspace : empty handle");
  }
}

void Workspaces::on_active_special(EventListener* /*event_listener*/, const char* const data)
{
  g_assert(GLib::MainContext::default_()->is_owner());
  spdlog::info("active special {}", data);
}
