#pragma once

#include "vin/iconfigurable.hpp"
#include "vin/ipositionable.hpp"
#include "vin/main_context.hpp"

#include <peel/GObject/ParamFlags.h>
#include <peel/class.h>
#include <peel/Gio/Initable.h>
#include <peel/GLib/MainContext.h>
#include <peel/GLib/Source.h>
#include <peel/Gtk/Box.h>
#include <peel/Gtk/Label.h>
#include <peel/Gtk/Widget.h>

#include <array>
#include <chrono>
#include <string_view>

namespace vin {

class TimeModule final : public peel::Gtk::Box
{
  PEEL_SIMPLE_CLASS(TimeModule, peel::Gtk::Box)

  friend class IPositionable;
  friend class IConfigurable;
  friend class peel::Gio::Initable;

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
  ~TimeModule() = default;

  static auto create(const MainContext context)
  {
    return peel::Object::create<TimeModule>(
      prop_main_context(), context.main_context, prop_worker_context(), context.worker_context);
  }

  VIN_MAIN_CONTEXT_PROPERTY

private:
  void init(Class* cls);

  static void init_type(peel::Type type);

  static void init_interface(IPositionable::Iface* iface);

  static void init_interface(IConfigurable::Iface* iface);

  static void init_interface(peel::Gio::Initable::Iface* iface);

  static int vin_time_module(lua_State* L);

  bool vfunc_init(peel::Gio::Cancellable* cancellable, peel::UniquePtr<peel::GLib::Error>* error);

  void vfunc_configure(lua_State* L);

  void vfunc_unconfigure(lua_State* L);

  void vfunc_position(Position position);

  void vfunc_dispose();

  void update_time();

  void update_format(std::string_view time_format, std::string_view date_format);

  static void define_properties(auto& visitor);
};

} // namespace vin
