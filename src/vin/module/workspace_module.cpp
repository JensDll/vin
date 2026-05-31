#include "vin/lib/hyprland_command.hpp"
#include "vin/lib/hyprland_workspace_list_model.hpp"
#include "vin/lib/iconfigurable.hpp"
#include "vin/lib/ipositionable.hpp"
#include "vin/module/workspace_module.hpp"

#include <fmt/format.h>
#include <peel/Gtk/ArrowType.h>
#include <peel/Gtk/BoolFilter.h>
#include <peel/Gtk/Button.h>
#include <peel/Gtk/FilterListModel.h>
#include <peel/Gtk/Label.h>
#include <peel/Gtk/ListItem.h>
#include <peel/Gtk/ListView.h>
#include <peel/Gtk/MenuButton.h>
#include <peel/Gtk/NoSelection.h>
#include <peel/Gtk/Orientable.h>
#include <peel/Gtk/Orientation.h>
#include <peel/Gtk/Popover.h>
#include <peel/Gtk/PropertyExpression.h>
#include <peel/Gtk/SignalListItemFactory.h>
#include <peel/Gtk/ToggleButton.h>
#include <peel/signal.h>
#include <spdlog/spdlog.h>

#include <utility>

extern "C" {
#include <lua.h>
}

using namespace peel;
using namespace vin::module;

PEEL_CLASS_IMPL(WorkspaceModule, "VinWorkspaceModule", Gtk::Box)

void WorkspaceModule::init([[maybe_unused]] Class* const cls)
{
  const auto workspaces{ lib::HyprlandWorkspaceListModel::create() };

  workspaces->connect_no_special(this, &WorkspaceModule::on_no_special);

  auto is_special_filter{ Gtk::BoolFilter::create(Gtk::PropertyExpression::create(
    Type::of<lib::HyprlandWorkspace>(), nullptr, lib::HyprlandWorkspace::prop_is_special().get_name())) };
  auto special_workspaces{ Gtk::FilterListModel::create(workspaces, std::move(is_special_filter)) };
  auto special_selection{ Gtk::NoSelection::create(std::move(special_workspaces)) };

  auto is_normal_filter{ Gtk::BoolFilter::create(Gtk::PropertyExpression::create(
    Type::of<lib::HyprlandWorkspace>(), nullptr, lib::HyprlandWorkspace::prop_is_normal().get_name())) };
  auto normal_workspaces{ Gtk::FilterListModel::create(workspaces, std::move(is_normal_filter)) };
  auto normal_selection{ Gtk::NoSelection::create(std::move(normal_workspaces)) };

  auto normal_factory{ Gtk::SignalListItemFactory::create() };
  normal_factory->connect_setup(this, &WorkspaceModule::on_setup);
  normal_factory->connect_bind(this, &WorkspaceModule::on_bind_normal);
  normal_factory->connect_unbind(this, &WorkspaceModule::on_unbind);

  auto special_factory{ Gtk::SignalListItemFactory::create() };
  special_factory->connect_setup(this, &WorkspaceModule::on_setup);
  special_factory->connect_bind(this, &WorkspaceModule::on_bind_special);
  special_factory->connect_unbind(this, &WorkspaceModule::on_unbind);

  auto special_view{ Gtk::ListView::create(std::move(special_selection), std::move(special_factory)) };
  special_view->set_name("special");
  m_special_view = special_view;

  auto normal_view{ Gtk::ListView::create(std::move(normal_selection), std::move(normal_factory)) };
  normal_view->set_name("normal");
  m_normal_view = normal_view;

  auto popover{ Gtk::Popover::create() };
  popover->set_child(std::move(special_view));
  popover->set_has_arrow(false);

  auto menu_button{ Gtk::MenuButton::create() };
  menu_button->set_sensitive(workspaces->num_special() > 0);
  menu_button->set_popover(std::move(popover));
  m_menu_button = menu_button;

  append(std::move(menu_button));
  append(std::move(normal_view));

  set_name("workspace-module");
}

void WorkspaceModule::Class::init() {}

void WorkspaceModule::init_type(const Type type)
{
  PEEL_IMPLEMENT_INTERFACE(type, lib::IPositionable);
  PEEL_IMPLEMENT_INTERFACE(type, lib::IConfigurable);
}

void WorkspaceModule::init_interface(lib::IPositionable::Iface* const iface)
{
  iface->override_vfunc_position<WorkspaceModule>();
}

void WorkspaceModule::init_interface(lib::IConfigurable::Iface* const iface)
{
  iface->override_vfunc_configure<WorkspaceModule>();
}

namespace {
void send_request(const std::string& request)
{
  UniquePtr<GLib::Error> error;
  const auto command{ vin::lib::HyprlandCommand::create(&error) };
  if (!command) {
    spdlog::error(error->message);
    return;
  }
  command->send_and_forget(request, nullptr);
}
} // namespace

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void WorkspaceModule::on_setup([[maybe_unused]] Gtk::SignalListItemFactory* const factory, Object* const obj)
{
  auto* const item{ obj->cast<Gtk::ListItem>() };
  item->set_child(Gtk::Button::create());
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void WorkspaceModule::on_bind_normal([[maybe_unused]] Gtk::SignalListItemFactory* const factory, Object* const obj)
{
  auto* const item{ obj->cast<Gtk::ListItem>() };
  auto* const button{ item->get_child()->cast<Gtk::Button>() };
  auto* const workspace{ item->get_item()->cast<lib::HyprlandWorkspace>() };

  button->set_child(Gtk::Label::create(workspace->id_str()));
  if (workspace->is_active()) {
    button->add_css_class("active");
  }

  workspace->register_for_dispose(workspace->connect_notify(
    lib::HyprlandWorkspace::prop_is_active(), [button](Object* const the_workspace, [[maybe_unused]] auto* const spec) {
      if (the_workspace->cast<lib::HyprlandWorkspace>()->is_active()) {
        button->add_css_class("active");
      } else {
        button->remove_css_class("active");
      }
    }));

  workspace->register_for_dispose(
    button->connect_clicked([request = fmt::format("dispatch hl.dsp.focus({{workspace={}}})", workspace->id())](
                              [[maybe_unused]] Gtk::Button* const the_button) {
      send_request(request);
    }));
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void WorkspaceModule::on_bind_special([[maybe_unused]] Gtk::SignalListItemFactory* const factory, Object* const obj)
{
  auto* const item{ obj->cast<Gtk::ListItem>() };
  auto* const button{ item->get_child()->cast<Gtk::Button>() };
  auto* const workspace{ item->get_item()->cast<lib::HyprlandWorkspace>() };

  button->set_child(Gtk::Label::create(workspace->special_name()));
  if (workspace->is_active()) {
    button->add_css_class("active");
  }

  m_menu_button->set_sensitive(true);

  workspace->register_for_dispose(workspace->connect_notify(
    lib::HyprlandWorkspace::prop_is_active(), [button](Object* const the_workspace, [[maybe_unused]] auto* const spec) {
      if (the_workspace->cast<lib::HyprlandWorkspace>()->is_active()) {
        button->add_css_class("active");
      } else {
        button->remove_css_class("active");
      }
    }));

  workspace->register_for_dispose(button->connect_clicked(
    [request = fmt::format("dispatch hl.dsp.workspace.toggle_special('{}')", workspace->special_name())](
      [[maybe_unused]] Gtk::Button* const the_button) {
      send_request(request);
    }));
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void WorkspaceModule::on_unbind([[maybe_unused]] Gtk::SignalListItemFactory* const factory, Object* const obj)
{
  auto* const item{ obj->cast<Gtk::ListItem>() };
  auto* const workspace{ item->get_item()->cast<lib::HyprlandWorkspace>() };
  workspace->dispose();
}

void WorkspaceModule::on_no_special([[maybe_unused]] lib::HyprlandWorkspaceListModel* const model)
{
  m_menu_button->set_sensitive(false);
}

void WorkspaceModule::vfunc_position(const lib::Position position)
{
  switch (position) {
  case lib::Position::top:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    m_normal_view->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    m_menu_button->set_direction(Gtk::ArrowType::DOWN);
    remove_css_class("top");
    remove_css_class("left");
    remove_css_class("right");
    remove_css_class("bottom");
    add_css_class("top");
    break;
  case lib::Position::left:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    m_normal_view->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    m_menu_button->set_direction(Gtk::ArrowType::RIGHT);
    remove_css_class("top");
    remove_css_class("left");
    remove_css_class("right");
    remove_css_class("bottom");
    add_css_class("left");
    break;
  case lib::Position::right:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    m_normal_view->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    m_menu_button->set_direction(Gtk::ArrowType::LEFT);
    remove_css_class("top");
    remove_css_class("left");
    remove_css_class("right");
    remove_css_class("bottom");
    add_css_class("right");
    break;
  case lib::Position::bottom:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    m_normal_view->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    m_menu_button->set_direction(Gtk::ArrowType::UP);
    remove_css_class("top");
    remove_css_class("left");
    remove_css_class("right");
    remove_css_class("bottom");
    add_css_class("bottom");
    break;
  }
}

void WorkspaceModule::vfunc_configure([[maybe_unused]] lua_State* const L) {}
