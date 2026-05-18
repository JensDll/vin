#include "vin/hyprland/button.hpp"

#include <peel/class.h>
#include <peel/Gtk/AccessibleRole.h>
#include <peel/Gtk/BinLayout.h>
#include <peel/Gtk/EventSequenceState.h>
#include <peel/Gtk/GestureClick.h>
#include <peel/Gtk/Widget.h>
#include <spdlog/spdlog.h>

using namespace peel;
using namespace vin::hyprland;

PEEL_CLASS_IMPL(Button, "VinHyprlandButton", Gtk::Widget)

Button::SignalClick Button::s_signal_click = Button::SignalClick::create("click");

void Button::init([[maybe_unused]] Class* const cls)
{
  set_focusable(true);
  set_receives_default(true);
  auto gesture_click{ Gtk::GestureClick::create() };
  gesture_click->connect_pressed(this, &Button::on_pressed);
  gesture_click->connect_released(this, &Button::on_released);
  add_controller(std::move(gesture_click));
}

void Button::Class::init()
{
  set_css_name("button");
  set_accessible_role(Gtk::Accessible::Role::BUTTON);
  set_layout_manager_type(Type::of<Gtk::BinLayout>());
  override_vfunc_dispose<Button>();
}

void Button::vfunc_dispose()
{
  spdlog::debug("dispose button");
  if (m_child != nullptr) {
    m_child->unparent();
    m_child = nullptr;
  }
  parent_vfunc_dispose<Gtk::Widget>();
}

void Button::set_child(peel::FloatPtr<peel::Gtk::Widget> child)
{
  if (child == m_child) {
    return;
  }

  if (m_child != nullptr) {
    m_child->unparent();
  }

  m_child = child;

  if (child != nullptr) {
    set_parent(std::move(child));
  }

  notify(prop_child());
}

void Button::on_pressed([[maybe_unused]] peel::Gtk::GestureClick* const gesture_click,
  [[maybe_unused]] const int n_press,
  [[maybe_unused]] const double x,
  [[maybe_unused]] const double y)
{
  if (get_focus_on_click() && !has_focus()) {
    grab_focus();
  }
}

void Button::on_released(peel::Gtk::GestureClick* const gesture_click,
  [[maybe_unused]] const int n_press,
  const double x,
  const double y)
{
  gesture_click->set_state(Gtk::EventSequenceState::CLAIMED);
  if (is_sensitive() && contains(x, y)) {
    s_signal_click.emit(this);
  }
}
