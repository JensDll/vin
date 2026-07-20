#pragma once

#include "vin/main_context.hpp"
#include "vin/notification.hpp"
#include "vin/notification_server.hpp"

#include <peel/GObject/Object.h>
#include <fmt/format.h>
#include <glibconfig.h>
#include <peel/class.h>
#include <peel/Gio/Initable.h>
#include <peel/Gio/ListModel.h>
#include <peel/RefPtr.h>
#include <peel/signal.h>
#include <spdlog/spdlog.h>

#include <vector>

namespace vin {

class NotificationListModel final : public peel::Gio::ListModel
{
  friend class peel::Gio::Initable;

  PEEL_SIMPLE_CLASS(NotificationListModel, peel::Gio::ListModel)

  using Empty = peel::Signal<NotificationListModel, void()>;
  using NotEmpty = peel::Signal<NotificationListModel, void()>;

  static guint32 s_next_id;
  static Empty s_empty;
  static NotEmpty s_not_empty;

  std::vector<peel::RefPtr<Notification>> m_notifications;

  peel::RefPtr<NotificationServer> m_server;

  peel::GLib::MainContext* m_main_context;
  peel::GLib::MainContext* m_worker_context;

public:
  ~NotificationListModel() = default;

  static peel::Type vfunc_get_item_type()
  {
    return peel::Type::of<Notification>();
  }

  [[nodiscard]] unsigned int vfunc_get_n_items() const
  {
    return m_notifications.size();
  }

  [[nodiscard]] peel::RefPtr<peel::Object> vfunc_get_item(const unsigned int pos) const
  {
    return pos < vfunc_get_n_items() ? m_notifications[pos] : nullptr;
  }

  void remove(guint pos);

  VIN_MAIN_CONTEXT_PROPERTY

  PEEL_SIGNAL_CONNECT_METHOD(empty, s_empty)
  PEEL_SIGNAL_CONNECT_METHOD(not_empty, s_not_empty)

private:
  static void init_type(peel::Type type);

  static void init_interface(peel::Gio::ListModel::Iface* iface);

  static void init_interface(peel::Gio::Initable::Iface* iface);

  bool vfunc_init(peel::Gio::Cancellable* cancellable, peel::UniquePtr<peel::GLib::Error>* error);

  void vfunc_dispose();

  void on_new_notification(NotificationServer* server, Notification* notification);

  static void dispose_notify(gpointer data, ::GObject* where_the_object_was);

  static void define_properties(auto& visitor);
};

} // namespace vin
