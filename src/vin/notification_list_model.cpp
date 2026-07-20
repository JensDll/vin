#include "vin/main_context.hpp"
#include "vin/notification.hpp"
#include "vin/notification_list_model.hpp"
#include "vin/notification_server.hpp"

#include <glib-object.h>
#include <peel/class.h>
#include <peel/Gio/Cancellable.h>
#include <peel/Gio/Initable.h>
#include <peel/GLib/Error.h>
#include <peel/RefPtr.h>
#include <peel/UniquePtr.h>
#include <peel/WeakPtr.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <iterator>

using namespace vin;
using namespace peel;

PEEL_CLASS_IMPL(NotificationListModel, "VinNotificationListModel", Object)

guint32 NotificationListModel::s_next_id{};

NotificationListModel::Empty NotificationListModel::s_empty;
NotificationListModel::NotEmpty NotificationListModel::s_not_empty;

void NotificationListModel::Class::init()
{
  s_empty = Empty::create("empty");
  s_not_empty = NotEmpty::create("not-empty");
}

void NotificationListModel::init_type(const Type type)
{
  PEEL_IMPLEMENT_INTERFACE(type, Gio::ListModel);
  PEEL_IMPLEMENT_INTERFACE(type, Gio::Initable);
}

void NotificationListModel::init_interface(Gio::ListModel::Iface* const iface)
{
  iface->override_vfunc_get_item_type<NotificationListModel>();
  iface->override_vfunc_get_n_items<NotificationListModel>();
  iface->override_vfunc_get_item<NotificationListModel>();
}

void NotificationListModel::init_interface(Gio::Initable::Iface* const iface)
{
  iface->override_vfunc_init<NotificationListModel>();
}

bool NotificationListModel::vfunc_init([[maybe_unused]] Gio::Cancellable* const cancellable,
  [[maybe_unused]] UniquePtr<GLib::Error>* const error)
{
  m_server = NotificationServer::create({ .main_context = m_main_context, .worker_context = m_worker_context });
  m_server->connect_new_notification(this, &NotificationListModel::on_new_notification);
  m_worker_context->invoke([this]() {
    m_server->start();
    return G_SOURCE_REMOVE;
  });
  return true;
}

namespace {
struct NotificationCompare
{
  static constexpr unsigned int get(const Notification* const value)
  {
    return value->get_id();
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

void NotificationListModel::dispose_notify(const gpointer data, [[maybe_unused]] ::GObject* const where_the_object_was)
{
  auto* const model{ static_cast<NotificationListModel*>(data) };
  if (model->m_notifications.empty()) {
    s_empty.emit(model);
  }
}

void NotificationListModel::remove(const guint pos)
{
  ::g_object_weak_ref(reinterpret_cast<::GObject*>(m_notifications[pos]->cast<peel::Object>()), dispose_notify, this);
  m_notifications.erase(m_notifications.cbegin() + pos);
  items_changed(pos, 1, 0);
}

void NotificationListModel::on_new_notification([[maybe_unused]] NotificationServer* const server,
  Notification* const notification)
{
  if (m_notifications.empty()) {
    s_not_empty.emit(this);
  }

  if (notification->get_replaces_id() == 0) {
    notification->set_id(++s_next_id);
    m_notifications.emplace_back(notification);
    items_changed(m_notifications.size() - 1, 0, 1);
    return;
  }

  const auto it{ std::ranges::lower_bound(m_notifications, notification->get_replaces_id(), NotificationCompare{}) };

  if (it == m_notifications.end() || (*it)->get_id() != notification->get_replaces_id()) {
    spdlog::warn(
      "New notification with replaces id {}, but no such notification exists", notification->get_replaces_id());
    return;
  }

  *it = notification;

  items_changed(std::distance(m_notifications.begin(), it), 0, 0);
}

VIN_IMPLEMENT_MAIN_CONTEXT_PROPERTY(NotificationListModel)

void NotificationListModel::define_properties(auto& visitor)
{
  VIN_DEFINE_MAIN_CONTEXT_PROPERTY(NotificationListModel)
}
