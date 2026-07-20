#pragma once

#include "vin/ipositionable.hpp"
#include "vin/main_context.hpp"
#include "vin/notification_list_model.hpp"

#include <peel/GObject/Object.h>
#include <peel/class.h>
#include <peel/Gio/Initable.h>
#include <peel/Gtk/ListView.h>
#include <peel/Gtk/SignalListItemFactory.h>
#include <peel/Gtk/Window.h>
#include <peel/RefPtr.h>
#include <peel/signal.h>

namespace vin {

class NotificationWindow final : public peel::Gtk::Window
{
  friend class IPositionable;
  friend class peel::Gio::Initable;

  PEEL_SIMPLE_CLASS(NotificationWindow, peel::Gtk::Window)

  NotificationListModel* m_model;
  peel::GLib::MainContext* m_main_context;
  peel::GLib::MainContext* m_worker_context;

public:
  ~NotificationWindow() = default;

  static auto create(const MainContext context)
  {
    return peel::Object::create<NotificationWindow>(
      prop_main_context(), context.main_context, prop_worker_context(), context.worker_context);
  }

  VIN_MAIN_CONTEXT_PROPERTY

private:
  void init(Class* cls);

  static void init_type(peel::Type type);

  static void init_interface(IPositionable::Iface* iface);

  static void init_interface(peel::Gio::Initable::Iface* iface);

  bool vfunc_init(peel::Gio::Cancellable* cancellable, peel::UniquePtr<peel::GLib::Error>* error);

  void on_setup(peel::Gtk::SignalListItemFactory* factory, peel::Object* obj);

  void on_bind(peel::Gtk::SignalListItemFactory* factory, peel::Object* obj);

  void on_unbind(peel::Gtk::SignalListItemFactory* factory, peel::Object* obj);

  void on_teardown(peel::Gtk::SignalListItemFactory* factory, peel::Object* obj);

  void on_activate(peel::Gtk::ListView* view, guint position);

  void on_empty(NotificationListModel* model);

  void on_not_empty(NotificationListModel* model);

  void vfunc_position(Position position);

  static void define_properties(auto& visitor);
};

} // namespace vin
