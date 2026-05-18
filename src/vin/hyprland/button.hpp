#pragma once

#include <peel/class.h>
#include <peel/FloatPtr.h>
#include <peel/Gtk/GestureClick.h>
#include <peel/Gtk/Label.h>
#include <peel/Gtk/Widget.h>
#include <peel/RefPtr.h>

namespace vin::hyprland {

class Button final : public peel::Gtk::Widget
{
  PEEL_SIMPLE_CLASS(Button, peel::Gtk::Widget)

  friend class peel::Gtk::Widget;

  using SignalClick = peel::Signal<Button, void()>;

  static SignalClick s_signal_click;

  peel::Gtk::Widget* m_child;

public:
  ~Button() = default;

  static auto create()
  {
    return peel::Object::create<Button>();
  }

  static auto create_with_label(const char* const label)
  {
    auto child{ peel::Gtk::Label::create(label) };
    return peel::Object::create<Button>(prop_child(), std::move(child));
  }

  PEEL_PROPERTY(peel::Gtk::Widget, child, "child")

  [[nodiscard]] peel::Gtk::Widget* get_child() const
  {
    return m_child;
  }

  void set_child(peel::FloatPtr<peel::Gtk::Widget> child);

  PEEL_SIGNAL_CONNECT_METHOD(click, s_signal_click)

private:
  static void define_properties(auto& visitor)
  {
    visitor.prop(prop_child()).get(&Button::get_child).set(&Button::set_child);
  }

  void init(Class* cls);

  void vfunc_dispose();

  void on_pressed(peel::Gtk::GestureClick* gesture_click, int n_press, double x, double y);

  void on_released(peel::Gtk::GestureClick* gesture_click, int n_press, double x, double y);
};

} // namespace vin::hyprland
