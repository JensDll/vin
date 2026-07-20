#include "vin/ipositionable.hpp"
#include "vin/notification.hpp"
#include "vin/notification_item.hpp"
#include "vin/notification_list_model.hpp"
#include "vin/notification_window.hpp"

#include <peel/GObject/Object.h>
#include <fmt/format.h>
#include <peel/class.h>
#include <peel/Gio/Initable.h>
#include <peel/Gtk/Align.h>
#include <peel/Gtk/Label.h>
#include <peel/Gtk/ListItem.h>
#include <peel/Gtk/ListView.h>
#include <peel/Gtk/NoSelection.h>
#include <peel/Gtk/Orientation.h>
#include <peel/Gtk/ScrolledWindow.h>
#include <peel/Gtk/SignalListItemFactory.h>
#include <peel/Gtk/SingleSelection.h>
#include <peel/Gtk4LayerShell/Gtk4LayerShell.h>
#include <peel/widget-template.h>
#include <spdlog/spdlog.h>

using namespace vin;
using namespace peel;

PEEL_CLASS_IMPL(NotificationWindow, "VinNotificationWindow", Window)

void NotificationWindow::Class::init()
{
  set_css_name("notification-window");
  set_template_from_resource("/com/doellmann/vin/notification_window.ui");
  PEEL_WIDGET_TEMPLATE_BIND_CHILD(NotificationWindow, m_model, "model");
  PEEL_WIDGET_TEMPLATE_BIND_CALLBACK(NotificationWindow, on_activate, "on_activate");
  PEEL_WIDGET_TEMPLATE_BIND_CALLBACK(NotificationWindow, on_empty, "on_empty");
  PEEL_WIDGET_TEMPLATE_BIND_CALLBACK(NotificationWindow, on_not_empty, "on_not_empty");
}

void NotificationWindow::init([[maybe_unused]] Class* const cls)
{
  Type::of<NotificationListModel>().ensure();
  Type::of<Notification>().ensure();
  Type::of<NotificationItem>().ensure();
  init_template();
  Gtk4LayerShell::init_for_window(this);
  Gtk4LayerShell::set_layer(this, Gtk4LayerShell::Layer::TOP);
  Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::TOP, true);
  Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::RIGHT, true);
  Gtk4LayerShell::set_margin(this, Gtk4LayerShell::Edge::TOP, 32);
  Gtk4LayerShell::set_margin(this, Gtk4LayerShell::Edge::RIGHT, 32);
  set_default_size(-1, -1);
}

void NotificationWindow::init_type(const Type type)
{
  PEEL_IMPLEMENT_INTERFACE(type, IPositionable);
  PEEL_IMPLEMENT_INTERFACE(type, Gio::Initable);
}

void NotificationWindow::init_interface(IPositionable::Iface* const iface)
{
  iface->override_vfunc_position<NotificationWindow>();
}

void NotificationWindow::init_interface(Gio::Initable::Iface* const iface)
{
  iface->override_vfunc_init<NotificationWindow>();
}

bool NotificationWindow::vfunc_init(Gio::Cancellable* const cancellable, UniquePtr<peel::GLib::Error>* const error)
{
  return m_model->cast<Gio::Initable>()->init(cancellable, error);
}

void NotificationWindow::on_activate([[maybe_unused]] peel::Gtk::ListView* const view, const guint position)
{
  m_model->remove(position);
  if (m_model->vfunc_get_n_items() != 0) {
    queue_resize();
  }
}

void NotificationWindow::on_empty([[maybe_unused]] NotificationListModel* const model)
{
  spdlog::info("hide notification window");
  set_visible(false);
}

void NotificationWindow::on_not_empty([[maybe_unused]] NotificationListModel* const model)
{
  spdlog::info("show notification window");
  present();
}

void NotificationWindow::vfunc_position(const Position position)
{
  spdlog::info("position {}", fmt::underlying(position));
  // switch (position) {
  // case lib::Position::top:
  //   set_halign(Gtk::Align::START);
  //   break;
  // case lib::Position::left:
  //   set_halign(Gtk::Align::START);
  //   break;
  // case lib::Position::right:
  //   set_halign(Gtk::Align::END);
  //   break;
  // case lib::Position::bottom:
  //   set_halign(Gtk::Align::END);
  //   break;
  // }
}

VIN_IMPLEMENT_MAIN_CONTEXT_PROPERTY(NotificationWindow)

void NotificationWindow::define_properties(auto& visitor)
{
  VIN_DEFINE_MAIN_CONTEXT_PROPERTY(NotificationWindow)
}
