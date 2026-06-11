#include "vin/lib/notification.hpp"
#include "vin/lib/notification_server.hpp"

#include <peel/GObject/Object.h>
#include <glibconfig.h>
#include <peel/class.h>
#pragma once

#include <peel/Gio/ListModel.h>
#include <peel/RefPtr.h>
#include <peel/signal.h>

#include <vector>

namespace vin::lib {

class NotificationListModel final : public peel::Gio::ListModel
{
private:
  PEEL_SIMPLE_CLASS(NotificationListModel, peel::Gio::ListModel)

  using SignalEmpty = peel::Signal<NotificationListModel, void()>;
  using SignalNotEmpty = peel::Signal<NotificationListModel, void()>;

  static guint32 s_next_id;
  static SignalEmpty s_signal_empty;
  static SignalNotEmpty s_signal_not_empty;

  std::vector<peel::RefPtr<Notification>> m_notifications;

  friend class peel::Gio::ListModel;

public:
  ~NotificationListModel() = default;

  static auto create()
  {
    return peel::Object::create<NotificationListModel>();
  }

  PEEL_SIGNAL_CONNECT_METHOD(empty, s_signal_empty)
  PEEL_SIGNAL_CONNECT_METHOD(not_empty, s_signal_not_empty)

private:
  void init(Class* cls);

  static void init_type(peel::Type type);

  static void init_interface(peel::Gio::ListModel::Iface* iface);

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

  void vfunc_dispose();

  void on_notification(NotificationServer* server, Notification* notification);
};

} // namespace vin::lib
