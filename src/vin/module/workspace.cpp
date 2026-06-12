#include "vin/lib/hyprland_workspace_list_model.hpp"
#include "vin/lib/iconfigurable.hpp"
#include "vin/lib/ipositionable.hpp"
#include "vin/module/workspace.hpp"
#include "vin/module/workspace_item.hpp"

#include <peel/Gio/Initable.h>
#include <peel/Gio/Resource.h>
#include <peel/Gio/ResourceLookupFlags.h>
#include <peel/Gtk/ArrowType.h>
#include <peel/Gtk/BoolFilter.h>
#include <peel/Gtk/BuilderCScope.h>
#include <peel/Gtk/BuilderListItemFactory.h>
#include <peel/Gtk/FilterListModel.h>
#include <peel/Gtk/ListView.h>
#include <peel/Gtk/MenuButton.h>
#include <peel/Gtk/NoSelection.h>
#include <peel/Gtk/Orientable.h>
#include <peel/Gtk/Orientation.h>
#include <peel/Gtk/Popover.h>
#include <peel/Gtk/PropertyExpression.h>
#include <peel/Gtk/ToggleButton.h>
#include <peel/signal.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <utility>

extern "C" {
#include <lua.h>
}

using namespace peel;
using namespace vin::module;

PEEL_CLASS_IMPL(Workspace, "VinModuleWorkspace", Gtk::Box)

void Workspace::Class::init()
{
  override_vfunc_dispose<Workspace>();
}

void Workspace::vfunc_dispose()
{
  spdlog::info("dispose workspace module");
  parent_vfunc_dispose<Workspace>();
}

bool Workspace::vfunc_init(peel::Gio::Cancellable* const cancellable, peel::UniquePtr<peel::GLib::Error>* const error)
{
  spdlog::info("init workspace module");

  const auto model{ lib::HyprlandWorkspaceListModel::create(cancellable, error, m_main_context, m_worker_context) };

  if (!model) {
    return false;
  }

  model->connect_new_special(this, &Workspace::on_new_special);
  model->connect_remove_special(this, &Workspace::on_remove_special);

  Type::of<WorkspaceItem>().ensure();

  auto is_normal{ Gtk::BoolFilter::create(Gtk::PropertyExpression::create(
    Type::of<lib::HyprlandWorkspace>(), nullptr, lib::HyprlandWorkspace::prop_is_normal().get_name())) };
  auto normal_model{ Gtk::FilterListModel::create(model, std::move(is_normal)) };
  auto normal_selection{ Gtk::NoSelection::create(std::move(normal_model)) };

  auto is_special{ Gtk::BoolFilter::create(Gtk::PropertyExpression::create(
    Type::of<lib::HyprlandWorkspace>(), nullptr, lib::HyprlandWorkspace::prop_is_special().get_name())) };
  auto special_model{ Gtk::FilterListModel::create(model, std::move(is_special)) };
  auto special_selection{ Gtk::NoSelection::create(std::move(special_model)) };

  const auto scope{ Gtk::Builder::CScope::create() };
  auto factory{ Gtk::BuilderListItemFactory::create_from_resource(scope, "/com/doellmann/vin/workspace_list_item.ui") };

  auto normal_view{ Gtk::ListView::create(std::move(normal_selection), factory) };
  normal_view->set_name("normal");
  m_normal_view = normal_view;

  auto special_view{ Gtk::ListView::create(std::move(special_selection), std::move(factory)) };
  special_view->set_name("special");
  m_special_view = special_view;

  auto popover{ Gtk::Popover::create() };
  popover->set_child(std::move(special_view));
  popover->set_has_arrow(false);

  auto menu_button{ Gtk::MenuButton::create() };
  menu_button->set_sensitive(model->num_special() > 0);
  menu_button->set_popover(std::move(popover));
  m_menu_button = menu_button;

  append(std::move(menu_button));
  append(std::move(normal_view));

  set_name("workspace-module");

  return true;
}

void Workspace::on_new_special([[maybe_unused]] lib::HyprlandWorkspaceListModel* const model,
  [[maybe_unused]] const std::size_t count)
{
  m_menu_button->set_sensitive(true);
}

void Workspace::on_remove_special([[maybe_unused]] lib::HyprlandWorkspaceListModel* const model,
  [[maybe_unused]] const std::size_t count)
{
  if (count == 0) {
    m_menu_button->set_sensitive(false);
  }
}

void Workspace::vfunc_position(const lib::Position position)
{
  switch (position) {
  case lib::Position::top:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    m_normal_view->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    m_menu_button->set_direction(Gtk::ArrowType::DOWN);
    lib::add_position_css_class<lib::Position::top>(this);
    break;
  case lib::Position::left:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    m_normal_view->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    m_menu_button->set_direction(Gtk::ArrowType::RIGHT);
    lib::add_position_css_class<lib::Position::left>(this);
    break;
  case lib::Position::right:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    m_normal_view->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    m_menu_button->set_direction(Gtk::ArrowType::LEFT);
    lib::add_position_css_class<lib::Position::right>(this);
    break;
  case lib::Position::bottom:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    m_normal_view->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    m_menu_button->set_direction(Gtk::ArrowType::UP);
    lib::add_position_css_class<lib::Position::bottom>(this);
    break;
  }
}

void Workspace::init_type(const Type type)
{
  PEEL_IMPLEMENT_INTERFACE(type, lib::IPositionable);
  PEEL_IMPLEMENT_INTERFACE(type, lib::IConfigurable);
  PEEL_IMPLEMENT_INTERFACE(type, Gio::Initable);
}

void Workspace::init_interface(lib::IPositionable::Iface* const iface)
{
  iface->override_vfunc_position<Workspace>();
}

void Workspace::init_interface([[maybe_unused]] lib::IConfigurable::Iface* const iface) {}

void Workspace::init_interface(Gio::Initable::Iface* const iface)
{
  iface->override_vfunc_init<Workspace>();
}
