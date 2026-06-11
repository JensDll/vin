#pragma once

#include "vin/lib/iconfigurable.hpp"
#include "vin/lib/ipositionable.hpp"

#include <peel/GObject/ParamFlags.h>
#include <peel/class.h>
#include <peel/GLib/MainContext.h>
#include <peel/GLib/Source.h>
#include <peel/Gtk/Box.h>
#include <peel/Gtk/Label.h>
#include <peel/Gtk/Orientable.h>
#include <peel/Gtk/Orientation.h>
#include <peel/Gtk/Widget.h>

#include <array>
#include <chrono>
#include <string_view>

extern "C" {
#include <lua.h>
}

namespace vin::module {

class Time final : public peel::Gtk::Box
{
private:
  PEEL_SIMPLE_CLASS(Time, peel::Gtk::Box)

  friend class lib::IPositionable;
  friend class lib::IConfigurable;

  std::array<char, 256> m_buffer;

  char* m_date_format;
  char* m_time;
  char* m_date;

  peel::GLib::MainContext* m_main_context;
  peel::GLib::MainContext* m_worker_context;

  const std::chrono::time_zone* m_time_zone;

  peel::Gtk::Label* m_time_label;
  peel::Gtk::Label* m_date_label;

  peel::RefPtr<peel::GLib::Source> m_timeout;

public:
  ~Time() = default;

  static auto create(peel::GLib::MainContext* const main_context, peel::GLib::MainContext* const worker_context)
  {
    return peel::Object::create<Time>(peel::Gtk::Orientable::prop_orientation(),
      peel::Gtk::Orientation::VERTICAL,
      prop_spacing(),
      0,
      prop_main_context(),
      main_context,
      prop_worker_context(),
      worker_context);
  }

private:
  void init(Class* cls);

  static void init_type(peel::Type type);

  static void init_interface(lib::IPositionable::Iface* iface);

  static void init_interface(lib::IConfigurable::Iface* iface);

  static int vin_time_module(lua_State* L);

  void vfunc_configure(lua_State* L);

  void vfunc_unconfigure(lua_State* L);

  void vfunc_position(lib::Position position);

  void vfunc_dispose();

  void update_time();

  void update_format(std::string_view time_format, std::string_view date_format);

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
    visitor.prop(prop_main_context()).set(&Time::set_main_context).flags(peel::GObject::ParamFlags::CONSTRUCT_ONLY);
    visitor.prop(prop_worker_context()).set(&Time::set_worker_context).flags(peel::GObject::ParamFlags::CONSTRUCT_ONLY);
  }
};

} // namespace vin::module
