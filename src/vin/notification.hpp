#pragma once

#include <peel/GObject/Object.h>
#include <peel/GObject/ParamFlags.h>
#include <glib.h>
#include <glibconfig.h>
#include <peel/class.h>
#include <peel/Gio/DBusInterfaceInfo.h>
#include <peel/GLib/Variant.h>
#include <peel/property.h>
#include <peel/signal.h>
#include <peel/UniquePtr.h>

#include <array>
#include <map>
#include <string_view>
#include <vector>

namespace vin {

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"

namespace detail {

static ::GDBusArgInfo get_capabilities_capabilities{
  .ref_count = -1,
  .name = "capabilities",
  .signature = "as",
  .annotations = nullptr,
};

static ::GDBusArgInfo notify_id{
  .ref_count = -1,
  .name = "id",
  .signature = "u",
  .annotations = nullptr,
};

static ::GDBusArgInfo notify_app_name{
  .ref_count = -1,
  .name = "app_name",
  .signature = "s",
  .annotations = nullptr,
};

static ::GDBusArgInfo notify_replaces_id{
  .ref_count = -1,
  .name = "replaces_id",
  .signature = "u",
  .annotations = nullptr,
};

static ::GDBusArgInfo notify_app_icon{
  .ref_count = -1,
  .name = "app_icon",
  .signature = "s",
  .annotations = nullptr,
};

static ::GDBusArgInfo notify_summary{
  .ref_count = -1,
  .name = "summary",
  .signature = "s",
  .annotations = nullptr,
};

static ::GDBusArgInfo notify_body{
  .ref_count = -1,
  .name = "body",
  .signature = "s",
  .annotations = nullptr,
};

static ::GDBusArgInfo notify_actions{
  .ref_count = -1,
  .name = "actions",
  .signature = "as",
  .annotations = nullptr,
};

static ::GDBusArgInfo notify_hints{
  .ref_count = -1,
  .name = "hints",
  .signature = "a{sv}",
  .annotations = nullptr,
};

static ::GDBusArgInfo notify_expire_timeout{
  .ref_count = -1,
  .name = "expire_timeout",
  .signature = "i",
  .annotations = nullptr,
};

static ::GDBusArgInfo close_notification_id{
  .ref_count = -1,
  .name = "id",
  .signature = "u",
  .annotations = nullptr,
};

static ::GDBusArgInfo get_server_information_name{
  .ref_count = -1,
  .name = "name",
  .signature = "s",
  .annotations = nullptr,
};

static ::GDBusArgInfo get_server_information_vendor{
  .ref_count = -1,
  .name = "vendor",
  .signature = "s",
  .annotations = nullptr,
};

static ::GDBusArgInfo get_server_information_version{
  .ref_count = -1,
  .name = "version",
  .signature = "s",
  .annotations = nullptr,
};

static ::GDBusArgInfo get_server_information_spec_version{
  .ref_count = -1,
  .name = "spec_version",
  .signature = "s",
  .annotations = nullptr,
};

static ::GDBusArgInfo notification_closed_id{
  .ref_count = -1,
  .name = "id",
  .signature = "u",
  .annotations = nullptr,
};

static ::GDBusArgInfo notification_closed_reason{
  .ref_count = -1,
  .name = "reason",
  .signature = "u",
  .annotations = nullptr,
};

static ::GDBusArgInfo action_invoked_id{
  .ref_count = -1,
  .name = "id",
  .signature = "u",
  .annotations = nullptr,
};

static ::GDBusArgInfo action_invoked_action_key{
  .ref_count = -1,
  .name = "action_key",
  .signature = "s",
  .annotations = nullptr,
};

static ::GDBusArgInfo activation_token_id{
  .ref_count = -1,
  .name = "id",
  .signature = "u",
  .annotations = nullptr,
};

static ::GDBusArgInfo activation_token_activation_token{
  .ref_count = -1,
  .name = "activation_token",
  .signature = "s",
  .annotations = nullptr,
};

static std::array<::GDBusArgInfo*, 2> get_capabilities_out_args{
  &get_capabilities_capabilities,
};

static std::array<::GDBusArgInfo*, 9> notify_in_args{
  &notify_app_name,
  &notify_replaces_id,
  &notify_app_icon,
  &notify_summary,
  &notify_body,
  &notify_actions,
  &notify_hints,
  &notify_expire_timeout,
};

static std::array<::GDBusArgInfo*, 2> notify_out_args{
  &notify_id,
};

static std::array<::GDBusArgInfo*, 2> close_notification_in_args{
  &close_notification_id,
};

static std::array<::GDBusArgInfo*, 5> get_server_information_out_args{
  &get_server_information_name,
  &get_server_information_vendor,
  &get_server_information_version,
  &get_server_information_spec_version,
};

static std::array<::GDBusArgInfo*, 3> notification_closed_args{
  &notification_closed_id,
  &notification_closed_reason,
};

static std::array<::GDBusArgInfo*, 3> action_invoked_args{
  &action_invoked_id,
  &action_invoked_action_key,
};

static std::array<::GDBusArgInfo*, 3> activation_token_args{
  &activation_token_id,
  &activation_token_activation_token,
};

static ::GDBusMethodInfo get_capabilities{
  .ref_count = -1,
  .name = "GetCapabilities",
  .in_args = nullptr,
  .out_args = reinterpret_cast<::GDBusArgInfo**>(&get_capabilities_out_args),
  .annotations = nullptr,
};

static ::GDBusMethodInfo notify{
  .ref_count = -1,
  .name = "Notify",
  .in_args = reinterpret_cast<::GDBusArgInfo**>(&notify_in_args),
  .out_args = reinterpret_cast<::GDBusArgInfo**>(&notify_out_args),
  .annotations = nullptr,
};

static ::GDBusMethodInfo close_notification{
  .ref_count = -1,
  .name = "CloseNotification",
  .in_args = reinterpret_cast<::GDBusArgInfo**>(&close_notification_in_args),
  .out_args = nullptr,
  .annotations = nullptr,
};

static ::GDBusMethodInfo get_server_information{
  .ref_count = -1,
  .name = "GetServerInformation",
  .in_args = nullptr,
  .out_args = reinterpret_cast<::GDBusArgInfo**>(&get_server_information_out_args),
  .annotations = nullptr,
};

static ::GDBusSignalInfo notification_closed{
  .ref_count = -1,
  .name = "NotificationClosed",
  .args = reinterpret_cast<::GDBusArgInfo**>(&notification_closed_args),
  .annotations = nullptr,
};

static ::GDBusSignalInfo action_invoked{
  .ref_count = -1,
  .name = "ActionInvoked",
  .args = reinterpret_cast<::GDBusArgInfo**>(&action_invoked_args),
  .annotations = nullptr,
};

static ::GDBusSignalInfo activation_token{
  .ref_count = -1,
  .name = "ActivationToken",
  .args = reinterpret_cast<::GDBusArgInfo**>(&activation_token_args),
  .annotations = nullptr,
};

static std::array<::GDBusMethodInfo*, 5> methods{
  &get_capabilities,
  &notify,
  &close_notification,
  &get_server_information,
};

static std::array<::GDBusSignalInfo*, 4> signals{
  &notification_closed,
  &action_invoked,
  &activation_token,
};

} // namespace detail

static ::GDBusInterfaceInfo s_freedesktop_notifications_interface_info{
  .ref_count = -1,
  .name = "org.freedesktop.Notifications",
  .methods = reinterpret_cast<::GDBusMethodInfo**>(&detail::methods),
  .signals = reinterpret_cast<::GDBusSignalInfo**>(&detail::signals),
  .properties = nullptr,
  .annotations = nullptr,
};

#pragma GCC diagnostic pop

// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

class Notification final : public peel::Object
{
public:
  enum class Urgency : unsigned char { normal, low, critical };

private:
  PEEL_SIMPLE_CLASS(Notification, peel::Object)

  std::map<std::string_view, std::string_view> m_actions;
  std::vector<char> m_buffer;
  std::string_view m_app_name;
  std::string_view m_app_icon;
  std::string_view m_summary;
  std::string_view m_body;
  std::string_view m_category_hint;
  std::string_view m_desktop_entry_hint;
  guint32 m_replaces_id;
  guint32 m_id;
  gint32 m_expire_timeout;
  Urgency m_urgency_hint;

public:
  ~Notification() = default;

  static auto create()
  {
    return peel::Object::create<Notification>();
  }

private:
  void init(Class* cls);

public:
  void from_variant(::GVariant* notification);

  static bool is_valid_type(::GVariant* variant);

  PEEL_PROPERTY(const char*, app_name, "app-name")

  [[nodiscard]] const char* get_app_name() const
  {
    return m_app_name.data();
  }

  [[nodiscard]] std::string_view get_app_name_view() const
  {
    return m_app_name;
  }

  [[nodiscard]] bool has_app_name() const
  {
    return !m_app_name.empty();
  }

  PEEL_PROPERTY(const char*, app_icon, "app-icon")

  [[nodiscard]] const char* get_app_icon() const
  {
    return m_app_icon.data();
  }

  [[nodiscard]] std::string_view get_app_icon_view() const
  {
    return m_app_name;
  }

  [[nodiscard]] bool has_app_icon() const
  {
    return !m_app_icon.empty();
  }

  PEEL_PROPERTY(const char*, summary, "summary")

  [[nodiscard]] const char* get_summary() const
  {
    return m_summary.data();
  }

  [[nodiscard]] std::string_view get_summary_view() const
  {
    return m_summary;
  }

  PEEL_PROPERTY(const char*, body, "body")

  [[nodiscard]] const char* get_body() const
  {
    return m_body.data();
  }

  [[nodiscard]] std::string_view get_body_view() const
  {
    return m_body;
  }

  [[nodiscard]] bool has_body() const
  {
    return !m_body.empty();
  }

  [[nodiscard]] guint32 get_replaces_id() const
  {
    return m_replaces_id;
  }

  [[nodiscard]] gint32 get_expire_timeout() const
  {
    return m_expire_timeout;
  }

  [[nodiscard]] Urgency get_urgency_hint() const
  {
    return m_urgency_hint;
  }

  [[nodiscard]] std::string_view get_category_hint() const
  {
    return m_category_hint;
  }

  [[nodiscard]] std::string_view get_desktop_entry_hint() const
  {
    return m_desktop_entry_hint;
  }

  [[nodiscard]] std::string_view get_action(const std::string_view key) const
  {
    const auto it{ m_actions.find(key) };
    return it == m_actions.end() ? "" : it->second;
  }

  void set_id(const guint32 id)
  {
    m_id = id;
  }

  [[nodiscard]] guint32 get_id() const
  {
    return m_id;
  }

private:
  static void define_properties(auto& visitor)
  {
    visitor.prop(prop_app_name(), nullptr).get(&Notification::get_app_name);
    visitor.prop(prop_app_icon(), nullptr).get(&Notification::get_app_icon);
    visitor.prop(prop_summary(), nullptr).get(&Notification::get_summary);
    visitor.prop(prop_body(), nullptr).get(&Notification::get_body);
  }
};

} // namespace vin
