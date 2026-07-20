#pragma once

#include "vin/iconfigurable.hpp"
#include "vin/ipositionable.hpp"
#include "vin/main_context.hpp"
#include "vin/notification_window.hpp"

#include <peel/GObject/ParamFlags.h>
#include <peel/class.h>
#include <peel/Gio/Initable.h>
#include <peel/GLib/MainContext.h>
#include <peel/Gtk/Box.h>
#include <peel/Gtk/Button.h>
#include <peel/Gtk/ListView.h>
#include <peel/Gtk/MenuButton.h>
#include <peel/Gtk/Orientable.h>
#include <peel/Gtk/Orientation.h>
#include <peel/RefPtr.h>

namespace vin {

class NotificationModule final : public peel::Gtk::Button
{
  PEEL_SIMPLE_CLASS(NotificationModule, peel::Gtk::Button)

  friend class IPositionable;
  friend class IConfigurable;
  friend class peel::Gio::Initable;

  peel::GLib::MainContext* m_main_context;
  peel::GLib::MainContext* m_worker_context;
  NotificationWindow* m_window;

public:
  ~NotificationModule() = default;

  static auto create(const MainContext context)
  {
    return peel::Object::create<NotificationModule>(
      prop_main_context(), context.main_context, prop_worker_context(), context.worker_context);
  }

  VIN_MAIN_CONTEXT_PROPERTY

private:
  void init(Class* cls);

  static void init_type(peel::Type type);

  static void init_interface(IPositionable::Iface* iface);

  static void init_interface(IConfigurable::Iface* iface);

  static void init_interface(peel::Gio::Initable::Iface* iface);

  bool vfunc_init(peel::Gio::Cancellable* cancellable, peel::UniquePtr<peel::GLib::Error>* error);

  void vfunc_dispose();

  void vfunc_configure(lua_State* L);

  void vfunc_position(Position position);

  static void define_properties(auto& visitor);

  static int vin_notification_module(lua_State* L);
};

} // namespace vin
