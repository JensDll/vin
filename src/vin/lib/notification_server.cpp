#include "vin/lib/error.hpp"
#include "vin/lib/notification.hpp"
#include "vin/lib/notification_server.hpp"

#include <peel/GLib/MainContext.h>
#include <peel/GLib/Variant.h>
#include <spdlog/spdlog.h>

#include <cstring>

using namespace vin::lib;
using namespace peel;

PEEL_CLASS_IMPL(NotificationServer, "VinLibNotificationServer", Object)

NotificationServer::SignalNotification NotificationServer::s_signal_notification =
  NotificationServer::SignalNotification::create("notification");

void NotificationServer::Class::init()
{
  override_vfunc_dispose<NotificationServer>();
  override_vfunc_constructor<NotificationServer>();
}

void NotificationServer::vfunc_dispose()
{
  if (m_bus_id != 0) {
    ::g_bus_unown_name(m_bus_id);
  }
  parent_vfunc_dispose<NotificationServer>();
}

RefPtr<Object> NotificationServer::vfunc_constructor(const Type type, const ArrayRef<Object::ConstructParam> params)
{
  static Object* self{};

  if (self == nullptr) {
    const auto new_self{ parent_vfunc_constructor<NotificationServer>(type, params) };
    new_self->add_weak_pointer(&self);
    return self = new_self;
  }

  return self;
}

void NotificationServer::init([[maybe_unused]] Class* const cls) {}

void NotificationServer::own_bus()
{
  m_bus_id = ::g_bus_own_name(G_BUS_TYPE_SESSION,
    "org.freedesktop.Notifications",
    G_BUS_NAME_OWNER_FLAGS_NONE,
    on_bus_aquire,
    on_name_aquire,
    on_name_lost,
    this,
    nullptr);
}

void NotificationServer::get_capabilities(::GDBusMethodInvocation* const invocation)
{
  spdlog::info("get capabilities");
  ::GVariantBuilder builder;
  ::g_variant_builder_init_static(&builder, G_VARIANT_TYPE_STRING_ARRAY);
  ::g_variant_builder_add(&builder, "s", "actions");
  ::g_variant_builder_add(&builder, "s", "body");
  ::g_dbus_method_invocation_return_value(invocation, ::g_variant_new("(as)", &builder));
}

void NotificationServer::get_server_information(::GDBusMethodInvocation* invocation)
{
  spdlog::info("get server information");
  ::g_dbus_method_invocation_return_value(invocation, ::g_variant_new("(ssss)", "name", "vendor", "0.1.0", "1.3"));
}

void NotificationServer::close_notification([[maybe_unused]] ::GDBusMethodInvocation* const invocation,
  [[maybe_unused]] ::GVariant* const variant)
{
  spdlog::info("close notification");
}

void NotificationServer::notify(::GDBusMethodInvocation* const invocation,
  ::GVariant* const variant,
  NotificationServer* const server)
{
  g_assert(GLib::MainContext::get_thread_default() != nullptr);

  spdlog::info("notify");

  if (!Notification::is_valid_type(variant)) {
    ::g_dbus_method_invocation_return_error_literal(
      invocation, s_quark, static_cast<int>(Error::notify_invalid_type), "notification has invalid type");
    return;
  }

  const auto notification{ Notification::create() };
  notification->from_variant(variant);

  server->m_main_context->invoke([invocation, server, notification]() {
    NotificationServer::s_signal_notification.emit(server, notification);
    spdlog::info("new notification with id {}", notification->id());
    ::g_dbus_method_invocation_return_value(invocation, ::g_variant_new("(u)", notification->id()));
    return G_SOURCE_REMOVE;
  });
}

void NotificationServer::bus_method_call([[maybe_unused]] ::GDBusConnection* const connection,
  [[maybe_unused]] const gchar* const sender,
  [[maybe_unused]] const gchar* const object_path,
  [[maybe_unused]] const gchar* const interface_name,
  [[maybe_unused]] const gchar* const method_name,
  ::GVariant* const parameters,
  ::GDBusMethodInvocation* const invocation,
  const gpointer data)
{
  spdlog::info("handle method call");
  if (std::strcmp(method_name, "GetServerInformation") == 0) {
    get_server_information(invocation);
    return;
  }
  if (std::strcmp(method_name, "GetCapabilities") == 0) {
    get_capabilities(invocation);
    return;
  }
  if (std::strcmp(method_name, "Notify") == 0) {
    notify(invocation, parameters, static_cast<NotificationServer*>(data));
    return;
  }
  if (std::strcmp(method_name, "CloseNotification") == 0) {
    close_notification(invocation, parameters);
    return;
  }
}

void NotificationServer::on_bus_aquire(::GDBusConnection* connection,
  [[maybe_unused]] const gchar* const name,
  [[maybe_unused]] const gpointer data)
{
  spdlog::info("bus aquired");
  const ::GDBusInterfaceVTable handlers = { .method_call = bus_method_call };
  ::GError* error{};
  if (::g_dbus_connection_register_object(connection,
        "/org/freedesktop/Notifications",
        &s_freedesktop_notifications_interface_info,
        &handlers,
        data,
        nullptr,
        &error)
      == 0) {
    spdlog::error("failed to register object for path /org/freedesktop/Notifications : {}", error->message);
    g_error_free(error);
  }
}

void NotificationServer::on_name_aquire([[maybe_unused]] ::GDBusConnection* const connection,
  [[maybe_unused]] const gchar* const name,
  [[maybe_unused]] const gpointer data)
{}

void NotificationServer::on_name_lost([[maybe_unused]] ::GDBusConnection* const connection,
  [[maybe_unused]] const gchar* const name,
  [[maybe_unused]] const gpointer data)
{}
