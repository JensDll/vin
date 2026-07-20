#include "vin/iconfigurable.hpp"
#include "vin/ipositionable.hpp"
#include "vin/lua.hpp"
#include "vin/main_context.hpp"
#include "vin/notification_module.hpp"
#include "vin/notification_window.hpp"

#include <peel/class.h>
#include <peel/Gio/Initable.h>
#include <peel/Gtk/Box.h>
#include <peel/Gtk/BuilderCScope.h>
#include <peel/Gtk/BuilderListItemFactory.h>
#include <peel/Gtk/Button.h>
#include <peel/Gtk/Label.h>
#include <peel/Gtk/ListItem.h>
#include <peel/Gtk/ListView.h>
#include <peel/Gtk/NoSelection.h>
#include <peel/Gtk/Orientation.h>
#include <peel/Gtk/PolicyType.h>
#include <peel/Gtk/ScrolledWindow.h>
#include <peel/Gtk/SignalListItemFactory.h>
#include <peel/Gtk/SingleSelection.h>
#include <peel/Gtk4LayerShell/Gtk4LayerShell.h>
#include <peel/RefPtr.h>
#include <spdlog/spdlog.h>

using namespace vin;
using namespace peel;

PEEL_CLASS_IMPL(NotificationModule, "VinNotificationModule", Gtk::Button)

void NotificationModule::Class::init()
{
  set_css_name("notification-module");
  override_vfunc_dispose<NotificationModule>();
}

void NotificationModule::init([[maybe_unused]] Class* const cls)
{
  set_icon_name("notifications-disabled-symbolic");
}

void NotificationModule::init_type(const Type type)
{
  PEEL_IMPLEMENT_INTERFACE(type, IPositionable);
  PEEL_IMPLEMENT_INTERFACE(type, IConfigurable);
  PEEL_IMPLEMENT_INTERFACE(type, Gio::Initable);
}

void NotificationModule::init_interface(IPositionable::Iface* const iface)
{
  iface->override_vfunc_position<NotificationModule>();
}

void NotificationModule::init_interface(IConfigurable::Iface* const iface)
{
  iface->override_vfunc_configure<NotificationModule>();
}

void NotificationModule::init_interface(Gio::Initable::Iface* const iface)
{
  iface->override_vfunc_init<NotificationModule>();
}

bool NotificationModule::vfunc_init(Gio::Cancellable* const cancellable, UniquePtr<GLib::Error>* const error)
{
  m_window = NotificationWindow::create({ .main_context = m_main_context, .worker_context = m_worker_context });
  return m_window->cast<Gio::Initable>()->init(cancellable, error);
}

void NotificationModule::vfunc_dispose()
{
  peel::RefTraits<NotificationWindow>::unref(m_window);
  parent_vfunc_dispose<NotificationModule>();
}

void NotificationModule::vfunc_position(const Position position)
{
  m_window->cast<IPositionable>()->position(position);
}

void NotificationModule::vfunc_configure(lua_State* const L)
{
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, vin_notification_module, 1);
  lua_setfield(L, -2, "notification_module");
}

int NotificationModule::vin_notification_module(lua_State* const L)
{
  VIN_LUA_EXPECT_NARGS(1);
  VIN_LUA_EXPECT_TABLE(1);

  auto* const module{ static_cast<NotificationModule*>(lua_touserdata(L, lua_upvalueindex(1))) };

  lua_getfield(L, 1, "margin");
  if (lua_istable(L, 2) != 0) {
    lua_getfield(L, 2, "top");
    lua_getfield(L, 2, "left");
    lua_getfield(L, 2, "right");
    lua_getfield(L, 2, "bottom");
    if (lua_isinteger(L, 3) != 0) {
      Gtk4LayerShell::set_margin(module->m_window, Gtk4LayerShell::Edge::TOP, lua_tointeger(L, 3));
    }
    if (lua_isinteger(L, 4) != 0) {
      Gtk4LayerShell::set_margin(module->m_window, Gtk4LayerShell::Edge::LEFT, lua_tointeger(L, 4));
    }
    if (lua_isinteger(L, 5) != 0) {
      Gtk4LayerShell::set_margin(module->m_window, Gtk4LayerShell::Edge::RIGHT, lua_tointeger(L, 5));
    }
    if (lua_isinteger(L, 6) != 0) {
      Gtk4LayerShell::set_margin(module->m_window, Gtk4LayerShell::Edge::BOTTOM, lua_tointeger(L, 6));
    }
    lua_pop(L, 4);
  }

  return 0;
}

VIN_IMPLEMENT_MAIN_CONTEXT_PROPERTY(NotificationModule)

void NotificationModule::define_properties(auto& visitor)
{
  VIN_DEFINE_MAIN_CONTEXT_PROPERTY(NotificationModule)
}
