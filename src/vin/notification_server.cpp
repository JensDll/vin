#include "vin/error.hpp"
#include "vin/main_context.hpp"
#include "vin/notification.hpp"
#include "vin/notification_server.hpp"

#include <fmt/std.h>
#include <gio/gio.h>
#include <peel/GLib/MainContext.h>
#include <peel/GLib/Variant.h>
#include <spdlog/spdlog.h>

#include <cstring>

using namespace vin;
using namespace peel;

PEEL_CLASS_IMPL(NotificationServer, "VinNotificationServer", Object)

NotificationServer::NewNotification NotificationServer::s_new_notification;

void NotificationServer::Class::init()
{
  s_new_notification = NotificationServer::NewNotification::create("new-notification");
  override_vfunc_dispose<NotificationServer>();
}

void NotificationServer::vfunc_dispose()
{
  spdlog::info("[VinNotificationServer] dispose");
  stop();
  parent_vfunc_dispose<NotificationServer>();
}

void NotificationServer::start()
{
  g_assert(GLib::MainContext::get_thread_default() != nullptr);
  m_bus_id = ::g_bus_own_name(G_BUS_TYPE_SESSION,
    "org.freedesktop.Notifications",
    G_BUS_NAME_OWNER_FLAGS_NONE,
    on_bus_aquire,
    on_name_aquire,
    on_name_lost,
    this,
    nullptr);
}

void NotificationServer::stop()
{
  if (m_bus_id != 0) {
    ::g_bus_unown_name(m_bus_id);
  }

  if (m_connection != nullptr) {
    if (m_object_id != 0) {
      ::g_dbus_connection_unregister_object(m_connection, m_object_id);
    }
    g_clear_object(&m_connection);
  }

  m_bus_id = 0;
  m_object_id = 0;
  m_is_running = false;
}

void NotificationServer::command_get_capabilities(::GDBusMethodInvocation* const invocation)
{
  ::GVariantBuilder builder;
  ::g_variant_builder_init_static(&builder, G_VARIANT_TYPE_STRING_ARRAY);
  ::g_variant_builder_add(&builder, "s", "actions");
  ::g_variant_builder_add(&builder, "s", "body");
  ::g_dbus_method_invocation_return_value(invocation, ::g_variant_new("(as)", &builder));
}

void NotificationServer::command_get_server_information(::GDBusMethodInvocation* invocation)
{
  ::g_dbus_method_invocation_return_value(invocation, ::g_variant_new("(ssss)", "name", "vendor", "0.1.0", "1.3"));
}

void NotificationServer::command_close_notification([[maybe_unused]] ::GDBusMethodInvocation* const invocation,
  [[maybe_unused]] ::GVariant* const variant)
{}

void NotificationServer::command_notify(::GDBusMethodInvocation* const invocation,
  ::GVariant* const variant,
  NotificationServer* const server)
{
  if (!Notification::is_valid_type(variant)) {
    ::g_dbus_method_invocation_return_error_literal(
      invocation, s_quark, static_cast<int>(Error::notify_invalid_type), "notification has invalid type");
    return;
  }

  const auto notification{ Notification::create() };
  notification->from_variant(variant);

  server->m_main_context->invoke([invocation, server, notification]() {
    NotificationServer::s_new_notification.emit(server, notification);
    ::g_dbus_method_invocation_return_value(invocation, ::g_variant_new("(u)", notification->get_id()));
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
  spdlog::info("handle method {}", method_name);
  if (std::strcmp(method_name, "GetServerInformation") == 0) {
    command_get_server_information(invocation);
    return;
  }
  if (std::strcmp(method_name, "GetCapabilities") == 0) {
    command_get_capabilities(invocation);
    return;
  }
  if (std::strcmp(method_name, "Notify") == 0) {
    command_notify(invocation, parameters, static_cast<NotificationServer*>(data));
    return;
  }
  if (std::strcmp(method_name, "CloseNotification") == 0) {
    command_close_notification(invocation, parameters);
    return;
  }
}

#define OBJECT_PATH "/org/freedesktop/Notifications"

void NotificationServer::on_bus_aquire(::GDBusConnection* connection,
  [[maybe_unused]] const gchar* const name,
  [[maybe_unused]] const gpointer data)
{
  g_assert(GLib::MainContext::get_thread_default() != nullptr);

  static const ::GDBusInterfaceVTable s_handlers = { .method_call = bus_method_call };

  auto* const server{ static_cast<NotificationServer*>(data) };

  ::GError* error{};

  server->m_object_id = ::g_dbus_connection_register_object(
    connection, OBJECT_PATH, &s_freedesktop_notifications_interface_info, &s_handlers, data, nullptr, &error);

  if (server->m_object_id == 0) {
    spdlog::error("Failed to register object at " OBJECT_PATH " : {}", error->message);
    g_error_free(error);
  }

  server->m_connection = g_object_ref(connection);
}

void NotificationServer::on_name_aquire([[maybe_unused]] ::GDBusConnection* const connection,
  [[maybe_unused]] const gchar* const name,
  [[maybe_unused]] const gpointer data)
{
  auto* const server{ static_cast<NotificationServer*>(data) };
  server->m_is_running = true;
}

void NotificationServer::on_name_lost([[maybe_unused]] ::GDBusConnection* const connection,
  [[maybe_unused]] const gchar* const name,
  [[maybe_unused]] const gpointer data)
{
  auto* const server{ static_cast<NotificationServer*>(data) };
  server->stop();
}

VIN_IMPLEMENT_MAIN_CONTEXT_PROPERTY(NotificationServer)

void NotificationServer::define_properties(auto& visitor)
{
  VIN_DEFINE_MAIN_CONTEXT_PROPERTY(NotificationServer)
}
