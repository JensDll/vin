#pragma once

#include "vin/main_context.hpp"
#include "vin/notification.hpp"

#include <peel/GObject/Object.h>
#include <peel/class.h>
#include <peel/GLib/MainContext.h>
#include <peel/RefPtr.h>
#include <peel/signal.h>

namespace vin {

class NotificationServer final : public peel::Object
{
private:
  PEEL_SIMPLE_CLASS(NotificationServer, peel::Object)

  using NewNotification = peel::Signal<NotificationServer, void(Notification*)>;

  static NewNotification s_new_notification;

  peel::GLib::MainContext* m_main_context;
  peel::GLib::MainContext* m_worker_context;

  ::GDBusConnection* m_connection;
  guint m_bus_id;
  guint m_object_id;
  bool m_is_running;

public:
  ~NotificationServer() = default;

  static auto create(const MainContext context)
  {
    return peel::Object::create<NotificationServer>(
      prop_main_context(), context.main_context, prop_worker_context(), context.worker_context);
  }

  void start();

  void stop();

  VIN_MAIN_CONTEXT_PROPERTY

  PEEL_SIGNAL_CONNECT_METHOD(new_notification, s_new_notification)

private:
  void vfunc_dispose();

  static void command_get_capabilities(::GDBusMethodInvocation* invocation);

  static void command_get_server_information(::GDBusMethodInvocation* invocation);

  static void command_close_notification(::GDBusMethodInvocation* invocation, ::GVariant* variant);

  static void command_notify(::GDBusMethodInvocation* invocation, ::GVariant* variant, NotificationServer* server);

  static void bus_method_call(::GDBusConnection* connection,
    const gchar* sender,
    const gchar* object_path,
    const gchar* interface_name,
    const gchar* method_name,
    ::GVariant* parameters,
    ::GDBusMethodInvocation* invocation,
    gpointer data);

  static void on_bus_aquire(::GDBusConnection* connection, const gchar* name, gpointer data);

  static void on_name_aquire(::GDBusConnection* connection, const gchar* name, gpointer data);

  static void on_name_lost(::GDBusConnection* connection, const gchar* name, gpointer data);

  static void define_properties(auto& visitor);
};

} // namespace vin
