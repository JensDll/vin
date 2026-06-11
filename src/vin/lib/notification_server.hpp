#pragma once

#include "vin/lib/notification.hpp"

#include <peel/GObject/Object.h>
#include <peel/class.h>
#include <peel/GLib/MainContext.h>
#include <peel/RefPtr.h>
#include <peel/signal.h>

namespace vin::lib {

class NotificationServer final : public peel::Object
{
private:
  PEEL_SIMPLE_CLASS(NotificationServer, peel::Object)

  using SignalNotification = peel::Signal<NotificationServer, void(Notification*)>;

  static SignalNotification s_signal_notification;

  peel::GLib::MainContext* m_main_context;
  peel::GLib::MainContext* m_worker_context;
  unsigned int m_bus_id;

public:
  ~NotificationServer() = default;

  static auto create(peel::GLib::MainContext* const main_context, peel::GLib::MainContext* const worker_context)
  {
    return peel::Object::create<NotificationServer>(
      prop_main_context(), main_context, prop_worker_context(), worker_context);
  }

  static auto singleton()
  {
    return peel::Object::create<NotificationServer>();
  }

  void own_bus();

  PEEL_SIGNAL_CONNECT_METHOD(notification, s_signal_notification)

private:
  void init(Class* cls);

  void vfunc_dispose();

  static peel::RefPtr<peel::Object> vfunc_constructor(peel::Type type,
    peel::ArrayRef<peel::Object::ConstructParam> params);

  static void get_capabilities(::GDBusMethodInvocation* invocation);

  static void get_server_information(::GDBusMethodInvocation* invocation);

  static void close_notification(::GDBusMethodInvocation* invocation, ::GVariant* variant);

  static void notify(::GDBusMethodInvocation* invocation, ::GVariant* variant, NotificationServer* server);

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

  PEEL_PROPERTY(peel::GLib::MainContext, main_context, "main-context")

  void set_main_context(peel::GLib::MainContext* const main_context)
  {
    m_main_context = main_context;
  }

  PEEL_PROPERTY(peel::GLib::MainContext, worker_context, "worker-context")

  void set_worker_context(peel::GLib::MainContext* const worker_context)
  {
    m_worker_context = worker_context;
  }

  static void define_properties(auto& visitor)
  {
    visitor.prop(prop_main_context())
      .set(&NotificationServer::set_main_context)
      .flags(peel::GObject::ParamFlags::CONSTRUCT_ONLY);
    visitor.prop(prop_worker_context())
      .set(&NotificationServer::set_worker_context)
      .flags(peel::GObject::ParamFlags::CONSTRUCT_ONLY);
  }
};

} // namespace vin::lib
