#include "vin/lib/notification.hpp"
#include "vin/lib/notification_list_model.hpp"
#include "vin/lib/notification_server.hpp"

#include <peel/RefPtr.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <iterator>

using namespace vin::lib;
using namespace peel;

PEEL_CLASS_IMPL(NotificationListModel, "VinLibNotificationListModel", Object)

guint32 NotificationListModel::s_next_id{};

NotificationListModel::SignalEmpty NotificationListModel::s_signal_empty;
NotificationListModel::SignalNotEmpty NotificationListModel::s_signal_not_empty;

void NotificationListModel::Class::init()
{
  s_signal_empty = SignalEmpty::create("empty");
  s_signal_not_empty = SignalNotEmpty::create("not-empty");
}

void NotificationListModel::init([[maybe_unused]] Class* const cls)
{
  const auto server{ lib::NotificationServer::singleton() };
  server->connect_notification(this, &NotificationListModel::on_notification);
}

namespace {
struct NotificationCompare
{
  static constexpr unsigned int get(const Notification* const value)
  {
    return value->id();
  }

  static constexpr unsigned int get(const unsigned int value)
  {
    return value;
  }

  bool operator()(const auto& a, const auto& b) const
  {
    return get(a) < get(b);
  }
};
} // namespace

void NotificationListModel::on_notification([[maybe_unused]] NotificationServer* const server,
  Notification* const notification)
{
  if (m_notifications.empty()) {
    s_signal_not_empty.emit(this);
  }

  if (notification->replaces_id() == 0) {
    notification->id() = ++s_next_id;
    m_notifications.emplace_back(notification);
    items_changed(m_notifications.size() - 1, 0, 1);
    return;
  }

  const auto it{ std::ranges::lower_bound(m_notifications, notification->replaces_id(), NotificationCompare{}) };

  if (it == m_notifications.end() || (*it)->id() != notification->replaces_id()) {
    spdlog::warn("new notification with replaces id, but no such notification exists");
    return;
  }

  *it = notification;
  items_changed(std::distance(m_notifications.begin(), it), 0, 0);
}

void NotificationListModel::init_type(const Type type)
{
  PEEL_IMPLEMENT_INTERFACE(type, Gio::ListModel);
}

void NotificationListModel::init_interface(Gio::ListModel::Iface* const iface)
{
  iface->override_vfunc_get_item_type<NotificationListModel>();
  iface->override_vfunc_get_n_items<NotificationListModel>();
  iface->override_vfunc_get_item<NotificationListModel>();
}
