#include "vin/notification.hpp"
#include "vin/notification_item.hpp"

#include <peel/GObject/Object.h>
#include <fmt/std.h>
#include <peel/GLib/functions.h>
#include <peel/Gtk/Align.h>
#include <peel/Gtk/EventSequenceState.h>
#include <peel/Gtk/GestureClick.h>
#include <peel/Gtk/Window.h>
#include <peel/Gtk4LayerShell/Gtk4LayerShell.h>
#include <peel/widget-template.h>
#include <spdlog/spdlog.h>

using namespace vin;
using namespace peel;

PEEL_CLASS_IMPL(NotificationItem, "VinNotificationItem", Gtk::Grid)

void NotificationItem::Class::init()
{
  set_css_name("notification");
  override_vfunc_dispose<NotificationItem>();
  set_template_from_resource("/com/doellmann/vin/notification_item.ui");
  PEEL_WIDGET_TEMPLATE_BIND_CHILD(NotificationItem, m_summary, "summary");
  PEEL_WIDGET_TEMPLATE_BIND_CHILD(NotificationItem, m_body, "body");
  PEEL_WIDGET_TEMPLATE_BIND_CHILD(NotificationItem, m_app_icon, "app_icon");
  PEEL_WIDGET_TEMPLATE_BIND_CHILD(NotificationItem, m_app_name, "app_name");
}

void NotificationItem::init([[maybe_unused]] Class* const cls)
{
  init_template();
  set_halign(Gtk::Align::END);
}

void NotificationItem::vfunc_dispose()
{
  spdlog::info("[VinNotificationItem] dispose");
  m_notification = nullptr;
  dispose_template(Type::of<NotificationItem>());
  parent_vfunc_dispose<NotificationItem>();
}

Notification* NotificationItem::get_notification()
{
  return m_notification;
}

void NotificationItem::set_notification(Notification* const notification)
{
  spdlog::info("set notification {}", fmt::ptr(notification));

  g_return_if_fail(notification == nullptr || notification->check_type<Notification>());

  if (m_notification == notification) {
    return;
  }

  m_notification = notification;

  if (notification != nullptr) {
    const auto escaped_summary{ GLib::markup_escape_text(m_notification->get_summary(), -1) };
    m_summary->set_markup(escaped_summary.c_str());

    if (m_notification->has_body()) {
      const auto escaped_body{ GLib::markup_escape_text(m_notification->get_body(), -1) };
      m_body->set_markup(escaped_body.c_str());
    } else {
      m_body->hide();
    }

    if (m_notification->has_app_icon()) {
      spdlog::info("notification app_icon = {}", m_notification->get_app_icon_view());
    } else {
      m_app_icon->hide();
    }

    if (m_notification->has_app_name()) {
      m_app_name->set_label(m_notification->get_app_name());
    } else {
      m_app_name->hide();
    }
  }

  notify(prop_notification());
}

void NotificationItem::define_properties(auto& visitor)
{
  visitor.prop(prop_notification()).get(&NotificationItem::get_notification).set(&NotificationItem::set_notification);
}
