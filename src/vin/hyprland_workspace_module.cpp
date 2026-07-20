#include "vin/hyprland_workspace.hpp"
#include "vin/hyprland_workspace_item.hpp"
#include "vin/hyprland_workspace_list_model.hpp"
#include "vin/hyprland_workspace_module.hpp"
#include "vin/iconfigurable.hpp"
#include "vin/ipositionable.hpp"
#include "vin/main_context.hpp"

#include <peel/class.h>
#include <peel/Gio/Cancellable.h>
#include <peel/Gio/Initable.h>
#include <peel/Gtk/ArrowType.h>
#include <peel/Gtk/Box.h>
#include <peel/Gtk/Orientable.h>
#include <peel/Gtk/Orientation.h>
#include <peel/UniquePtr.h>
#include <peel/widget-template.h>
#include <spdlog/spdlog.h>

using namespace peel;
using namespace vin;

PEEL_CLASS_IMPL(HyprlandWorkspaceModule, "VinHyprlandWorkspaceModule", Gtk::Box)

void HyprlandWorkspaceModule::Class::init()
{
  set_css_name("workspace-module");
  override_vfunc_dispose<HyprlandWorkspaceModule>();
  set_template_from_resource("/com/doellmann/vin/hyprland_workspace_module.ui");
  PEEL_WIDGET_TEMPLATE_BIND_CHILD(HyprlandWorkspaceModule, m_model, "workspace_model");
  PEEL_WIDGET_TEMPLATE_BIND_CHILD(HyprlandWorkspaceModule, m_normal_view, "normal_view");
  PEEL_WIDGET_TEMPLATE_BIND_CHILD(HyprlandWorkspaceModule, m_menu_button, "menu_button");
}

void HyprlandWorkspaceModule::init([[maybe_unused]] Class* const cls)
{
  Type::of<HyprlandWorkspaceItem>().ensure();
  Type::of<HyprlandWorkspace>().ensure();
  Type::of<HyprlandWorkspaceListModel>().ensure();
  init_template();
}

void HyprlandWorkspaceModule::init_type(const Type type)
{
  PEEL_IMPLEMENT_INTERFACE(type, Gio::Initable);
  PEEL_IMPLEMENT_INTERFACE(type, IPositionable);
  PEEL_IMPLEMENT_INTERFACE(type, IConfigurable);
}

void HyprlandWorkspaceModule::init_interface(IPositionable::Iface* const iface)
{
  iface->override_vfunc_position<HyprlandWorkspaceModule>();
}

void HyprlandWorkspaceModule::init_interface(IConfigurable::Iface* const iface)
{
  iface->override_vfunc_configure<HyprlandWorkspaceModule>();
}

void HyprlandWorkspaceModule::init_interface(Gio::Initable::Iface* const iface)
{
  iface->override_vfunc_init<HyprlandWorkspaceModule>();
}

bool HyprlandWorkspaceModule::vfunc_init(Gio::Cancellable* const cancellable, UniquePtr<GLib::Error>* error)
{
  return m_model->cast<Gio::Initable>()->init(cancellable, error);
}

void HyprlandWorkspaceModule::vfunc_dispose()
{
  spdlog::info("[VinHyprlandWorkspaceModule] dispose");
  dispose_template(Type::of<HyprlandWorkspaceModule>());
  parent_vfunc_dispose<HyprlandWorkspaceModule>();
}

void HyprlandWorkspaceModule::vfunc_position(const Position position)
{
  switch (position) {
  case Position::top:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    m_normal_view->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    m_menu_button->set_direction(Gtk::ArrowType::DOWN);
    add_position_css_class<Position::top>(this);
    break;
  case Position::left:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    m_normal_view->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    m_menu_button->set_direction(Gtk::ArrowType::RIGHT);
    add_position_css_class<Position::left>(this);
    break;
  case Position::right:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    m_normal_view->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    m_menu_button->set_direction(Gtk::ArrowType::LEFT);
    add_position_css_class<Position::right>(this);
    break;
  case Position::bottom:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    m_normal_view->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    m_menu_button->set_direction(Gtk::ArrowType::UP);
    add_position_css_class<Position::bottom>(this);
    break;
  }
}

void HyprlandWorkspaceModule::vfunc_configure([[maybe_unused]] lua_State* const L) {}

VIN_IMPLEMENT_MAIN_CONTEXT_PROPERTY(HyprlandWorkspaceModule)

void HyprlandWorkspaceModule::define_properties(auto& visitor)
{
  VIN_DEFINE_MAIN_CONTEXT_PROPERTY(HyprlandWorkspaceModule)
}
