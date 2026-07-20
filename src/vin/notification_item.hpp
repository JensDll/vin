#pragma once

#include "vin/notification.hpp"

#include <peel/GObject/Object.h>
#include <peel/class.h>
#include <peel/Gtk/Grid.h>
#include <peel/Gtk/Image.h>
#include <peel/Gtk/Label.h>
#include <peel/Gtk/Window.h>
#include <peel/RefPtr.h>
#include <peel/signal.h>
#include <spdlog/spdlog.h>

namespace vin {

class NotificationItem final : public peel::Gtk::Grid
{
  PEEL_SIMPLE_CLASS(NotificationItem, peel::Gtk::Grid)

  peel::RefPtr<Notification> m_notification;
  peel::Gtk::Label* m_summary;
  peel::Gtk::Label* m_body;
  peel::Gtk::Label* m_app_name;
  peel::Gtk::Image* m_app_icon;

public:
  ~NotificationItem() = default;

  static auto create()
  {
    return peel::Object::create<NotificationItem>();
  }

  Notification* get_notification();

  void set_notification(Notification* notification);

  PEEL_PROPERTY(Notification, notification, "notification");

private:
  void init(Class* cls);

  void vfunc_dispose();

  static void define_properties(auto& visitor);
};

} // namespace vin
