#pragma once

#include "vin/application.hpp"
#include "vin/lib/iconfigurable.hpp"
#include "vin/lib/ipositionable.hpp"

#include <peel/GObject/Object.h>
#include <peel/class.h>
#include <peel/GLib/MainContext.h>
#include <peel/Gtk/ApplicationWindow.h>
#include <peel/Gtk/Box.h>
#include <peel/Gtk/CenterBox.h>
#include <peel/property.h>
#include <peel/RefPtr.h>

#include <vector>

extern "C" {
#include <lua.h>
}

namespace vin {

class Window final : public peel::Gtk::ApplicationWindow
{
private:
  PEEL_SIMPLE_CLASS(Window, peel::Gtk::ApplicationWindow)

  friend class lib::IPositionable;
  friend class lib::IConfigurable;

  std::vector<peel::Gtk::Widget*> m_left_modules;
  std::vector<peel::Gtk::Widget*> m_center_modules;
  std::vector<peel::Gtk::Widget*> m_right_modules;

  peel::Gtk::CenterBox* m_layout;

  peel::Gtk::Box* m_left_box;
  peel::Gtk::Box* m_center_box;
  peel::Gtk::Box* m_right_box;

  peel::GLib::MainContext* m_main_context;
  peel::GLib::MainContext* m_worker_context;

public:
  ~Window() = default;

  static auto create(Application* const app,
    peel::GLib::MainContext* const main_context,
    peel::GLib::MainContext* const worker_context)
  {
    return peel::Object::create<Window>(
      prop_application(), app, prop_main_context(), main_context, prop_worker_context(), worker_context);
  }

private:
  void init(Class* cls);

  static void init_type(peel::Type type);

  static void init_interface(lib::IPositionable::Iface* iface);

  static void init_interface(lib::IConfigurable::Iface* iface);

  void vfunc_position(lib::Position position);

  void vfunc_configure(lua_State* L);

  static int vin_window(lua_State* L);

  PEEL_PROPERTY(peel::GLib::MainContext, main_context, "main-context")

  void set_main_context(peel::GLib::MainContext* const main_context)
  {
    m_main_context = main_context;
  }

  PEEL_PROPERTY(peel::GLib::MainContext, worker_context, "worker-context")

  void set_worker_context(peel::GLib::MainContext* const worker_context)
  {
    m_worker_context = worker_context;
  }

  static void define_properties(auto& visitor)
  {
    visitor.prop(prop_main_context()).set(&Window::set_main_context).flags(peel::GObject::ParamFlags::CONSTRUCT_ONLY);
    visitor.prop(prop_worker_context())
      .set(&Window::set_worker_context)
      .flags(peel::GObject::ParamFlags::CONSTRUCT_ONLY);
  }
};

} // namespace vin
