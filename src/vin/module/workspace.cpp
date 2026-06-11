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
  spdlog::info("dispose workspace modulde");
  parent_vfunc_dispose<Workspace>();
}

bool Workspace::vfunc_init(peel::Gio::Cancellable* const cancellable, peel::UniquePtr<peel::GLib::Error>* const error)
{
  spdlog::info("init workspace module");

  const auto model{ lib::HyprlandWorkspaceListModel::create(cancellable, error, m_main_context, m_worker_context) };

  if (!model) {
    return false;
  }

  Type::of<WorkspaceItem>().ensure();

  auto is_normal{ Gtk::BoolFilter::create(Gtk::PropertyExpression::create(
    Type::of<lib::HyprlandWorkspace>(), nullptr, lib::HyprlandWorkspace::prop_is_normal().get_name())) };
  auto normal_model{ Gtk::FilterListModel::create(model, std::move(is_normal)) };
  auto normal_selection{ Gtk::NoSelection::create(std::move(normal_model)) };

  const auto scope{ Gtk::Builder::CScope::create() };
  auto factory{ Gtk::BuilderListItemFactory::create_from_resource(scope, "/com/doellmann/vin/workspace_list_item.ui") };

  auto normal_view{ Gtk::ListView::create(std::move(normal_selection), std::move(factory)) };
  normal_view->set_name("normal");
  m_normal_view = normal_view;

  append(std::move(normal_view));

  set_name("workspace-module");

  return true;
}

void Workspace::on_no_special([[maybe_unused]] lib::HyprlandWorkspaceListModel* const model)
{
  spdlog::info("no special");
  // m_menu_button->set_sensitive(false);
}

void Workspace::vfunc_position(const lib::Position position)
{
  switch (position) {
  case lib::Position::top:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    m_normal_view->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    // m_menu_button->set_direction(Gtk::ArrowType::DOWN);
    lib::add_position_css_class<lib::Position::top>(this);
    break;
  case lib::Position::left:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    m_normal_view->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    // m_menu_button->set_direction(Gtk::ArrowType::RIGHT);
    lib::add_position_css_class<lib::Position::left>(this);
    break;
  case lib::Position::right:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    m_normal_view->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    // m_menu_button->set_direction(Gtk::ArrowType::LEFT);
    lib::add_position_css_class<lib::Position::right>(this);
    break;
  case lib::Position::bottom:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    m_normal_view->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    // m_menu_button->set_direction(Gtk::ArrowType::UP);
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
