#include "vin/iconfigurable.hpp"
#include "vin/ipositionable.hpp"
#include "vin/lua.hpp"
#include "vin/main_context.hpp"
#include "vin/time_module.hpp"

#include <fmt/args.h>
#include <fmt/base.h>
#include <fmt/chrono.h>
#include <fmt/std.h>
#include <peel/class.h>
#include <peel/Gio/Cancellable.h>
#include <peel/Gio/Initable.h>
#include <peel/GLib/Error.h>
#include <peel/GLib/functions.h>
#include <peel/Gtk/Align.h>
#include <peel/Gtk/Label.h>
#include <peel/Gtk/Orientable.h>
#include <peel/Gtk/Orientation.h>
#include <peel/Gtk/Widget.h>
#include <peel/UniquePtr.h>
#include <peel/widget-template.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <string_view>

using namespace vin;
using namespace peel;

PEEL_CLASS_IMPL(TimeModule, "VinTimeModule", Gtk::Box)

void TimeModule::Class::init()
{
  set_css_name("time-module");
  set_template_from_resource("/com/doellmann/vin/time_module.ui");
  PEEL_WIDGET_TEMPLATE_BIND_CHILD(TimeModule, m_time_label, "time_label");
  PEEL_WIDGET_TEMPLATE_BIND_CHILD(TimeModule, m_date_label, "date_label");
  override_vfunc_dispose<TimeModule>();
}

void TimeModule::init_type(const Type type)
{
  PEEL_IMPLEMENT_INTERFACE(type, IPositionable);
  PEEL_IMPLEMENT_INTERFACE(type, IConfigurable);
  PEEL_IMPLEMENT_INTERFACE(type, Gio::Initable);
}

void TimeModule::init_interface(IPositionable::Iface* const iface)
{
  iface->override_vfunc_position<TimeModule>();
}

void TimeModule::init_interface(IConfigurable::Iface* const iface)
{
  iface->override_vfunc_configure<TimeModule>();
  iface->override_vfunc_unconfigure<TimeModule>();
}

void TimeModule::init_interface(Gio::Initable::Iface* const iface)
{
  iface->override_vfunc_init<TimeModule>();
}

constexpr auto DEFAULT_TIME_FORMAT{ "{:%H:%M}" };
constexpr auto DEFAULT_DATE_FORMAT{ "{:%a|%d|%m|%y}" };

void TimeModule::init([[maybe_unused]] Class* const cls)
{
  init_template();

  m_time_zone = std::chrono::current_zone();

  update_format(DEFAULT_TIME_FORMAT, DEFAULT_DATE_FORMAT);
  update_time();

  m_time_label->set_label(m_time);
  m_date_label->set_label(m_date);

  m_timeout = GLib::timeout_source_new(500);

  m_timeout->set_callback([this]() {
    update_time();
    m_time_label->set_label(m_time);
    m_date_label->set_label(m_date);
    return G_SOURCE_CONTINUE;
  });
}

bool TimeModule::vfunc_init([[maybe_unused]] Gio::Cancellable* const cancellable,
  [[maybe_unused]] UniquePtr<GLib::Error>* const error)
{
  m_timeout->attach(m_worker_context);
  return true;
}

void TimeModule::vfunc_dispose()
{
  spdlog::info("[VinTimeModule] dispose");
  dispose_template(Type::of<TimeModule>());
  m_timeout->destroy();
  m_timeout = nullptr;
  parent_vfunc_dispose<TimeModule>();
}

void TimeModule::update_time()
{
  const auto now{ std::chrono::system_clock::now() };
  const auto time{ m_time_zone->to_local(now) };
  const fmt::basic_format_arg<fmt::context> arg(time);
  const fmt::basic_format_args<fmt::context> args(&arg, 1);
  try {
    auto result{ fmt::vformat_to_n(
      m_time, std::distance(m_time, m_buffer.end() - 2), std::string_view{ m_buffer.data(), m_date_format }, args) };
    *result.out = '\0';
    m_date = result.out + 1;
    result = fmt::vformat_to_n(
      m_date, std::distance(m_date, m_buffer.end() - 1), std::string_view{ m_date_format, m_time }, args);
    *result.out = '\0';
  } catch (const fmt::format_error& err) {
    spdlog::error("{}", err);
  }
}

void TimeModule::update_format(const std::string_view time_format, const std::string_view date_format)
{
  if (time_format.size() + date_format.size() > m_buffer.size() / 4) {
    spdlog::error("time_format and date_format are too long, using defaults");
    update_format(DEFAULT_TIME_FORMAT, DEFAULT_DATE_FORMAT);
    return;
  }
  m_date_format = m_buffer.data() + time_format.size();
  m_time = m_date_format + date_format.size();
  std::memcpy(m_date_format, date_format.data(), date_format.size());
  std::memcpy(m_buffer.data(), time_format.data(), time_format.size());
}

void TimeModule::vfunc_position(const Position position)
{
  switch (position) {
  case Position::top:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    add_position_css_class<Position::top>(this);
    break;
  case Position::left:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    add_position_css_class<Position::left>(this);
    break;
  case Position::right:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    add_position_css_class<Position::right>(this);
    break;
  case Position::bottom:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    add_position_css_class<Position::bottom>(this);
    break;
  }
}

int TimeModule::vin_time_module(lua_State* const L)
{
  VIN_LUA_EXPECT_NARGS(1);
  VIN_LUA_EXPECT_TABLE(1);

  auto* const module{ static_cast<TimeModule*>(lua_touserdata(L, lua_upvalueindex(1))) };

  lua_getfield(L, 1, "time_format");
  lua_getfield(L, 1, "date_format");

  std::string_view time_format{ module->m_buffer.data(), module->m_date_format };
  std::string_view date_format{ module->m_date_format, module->m_time };

  std::size_t size{};

  if (lua_isstring(L, 2) != 0) {
    const char* const data{ lua_tolstring(L, 2, &size) };
    time_format = { data, size };
  }

  if (lua_isstring(L, 3) != 0) {
    const char* const data{ lua_tolstring(L, 3, &size) };
    date_format = { data, size };
  }

  module->update_format(time_format, date_format);
  module->update_time();
  module->m_time_label->set_label(module->m_time);
  module->m_date_label->set_label(module->m_date);

  VIN_LUA_ASSERT_POP(L, 3);

  return 0;
}

void TimeModule::vfunc_configure(lua_State* const L)
{
  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, vin_time_module, 1);
  lua_setfield(L, -2, "time_module");
}

void TimeModule::vfunc_unconfigure(lua_State* const L)
{
  lua_pushnil(L);
  lua_setfield(L, -2, "time_module");
}

VIN_IMPLEMENT_MAIN_CONTEXT_PROPERTY(TimeModule)

void TimeModule::define_properties(auto& visitor)
{
  VIN_DEFINE_MAIN_CONTEXT_PROPERTY(TimeModule)
}
