#include "vin/lib/iconfigurable.hpp"
#include "vin/lib/ipositionable.hpp"
#include "vin/module/time_module.hpp"

#include <fmt/args.h>
#include <fmt/base.h>
#include <fmt/chrono.h>
#include <fmt/std.h>
#include <peel/class.h>
#include <peel/GLib/functions.h>
#include <peel/Gtk/Align.h>
#include <peel/Gtk/Label.h>
#include <peel/Gtk/Widget.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <string_view>
#include <utility>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
}

using namespace vin::module;
using namespace peel;

PEEL_CLASS_IMPL(TimeModule, "VinTimeModule", Gtk::Box)

void TimeModule::Class::init()
{
  override_vfunc_dispose<TimeModule>();
}

void TimeModule::vfunc_dispose()
{
  m_timeout->destroy();
  m_timeout = nullptr;
  parent_vfunc_dispose<TimeModule>();
}

void TimeModule::init_type(const Type type)
{
  PEEL_IMPLEMENT_INTERFACE(type, lib::IPositionable);
  PEEL_IMPLEMENT_INTERFACE(type, lib::IConfigurable);
}

void TimeModule::init_interface(lib::IPositionable::Iface* const iface)
{
  iface->override_vfunc_position<TimeModule>();
}

void TimeModule::init_interface(lib::IConfigurable::Iface* const iface)
{
  iface->override_vfunc_configure<TimeModule>();
}

void TimeModule::update_time()
{
  const auto now{ std::chrono::system_clock::now() };
  const auto time{ m_time_zone->to_local(now) };
  fmt::dynamic_format_arg_store<fmt::format_context> store;
  store.push_back(time);
  try {
    auto result{ fmt::vformat_to_n(
      m_time, std::distance(m_time, m_buffer.end() - 2), std::string_view{ m_buffer.data(), m_date_format }, store) };
    *result.out = '\0';
    m_date = result.out + 1;
    result = fmt::vformat_to_n(
      m_date, std::distance(m_date, m_buffer.end() - 1), std::string_view{ m_date_format, m_time }, store);
    *result.out = '\0';
  } catch (const fmt::format_error& err) {
    spdlog::error("{}", err);
  }
}

void TimeModule::update_format(const std::string_view time_format, const std::string_view date_format)
{
  if (time_format.size() + date_format.size() > m_buffer.size() / 4) {
    spdlog::error("time_format and date_format are too long, using defaults");
    update_format("{:%H:%M}", "{:%d/%m/%y}");
    return;
  }
  m_date_format = m_buffer.data() + time_format.size();
  m_time = m_date_format + date_format.size();
  std::memcpy(m_date_format, date_format.data(), date_format.size());
  std::memcpy(m_buffer.data(), time_format.data(), time_format.size());
}

void TimeModule::init([[maybe_unused]] Class* const cls)
{
  m_time_zone = std::chrono::current_zone();

  update_format("{:%H:%M}", "{:%d/%m/%y}");

  auto time_label{ Gtk::Label::create(nullptr) };
  time_label->set_name("time-label");
  m_time_label = time_label;

  auto date_label{ Gtk::Label::create(nullptr) };
  date_label->set_name("date-label");
  m_date_label = date_label;

  m_timeout = GLib::timeout_source_new(500);

  m_timeout->set_callback([this]() {
    update_time();
    m_time_label->set_label(m_time);
    m_date_label->set_label(m_date);
    return G_SOURCE_CONTINUE;
  });

  m_timeout->attach(m_worker_context);

  append(std::move(time_label));
  append(std::move(date_label));

  set_name("time-module");
}

void TimeModule::vfunc_position(const lib::Position position)
{
  switch (position) {
  case lib::Position::top:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    lib::add_position_css_class<lib::Position::top>(this);
    break;
  case lib::Position::left:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    lib::add_position_css_class<lib::Position::left>(this);
    break;
  case lib::Position::right:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    lib::add_position_css_class<lib::Position::right>(this);
    break;
  case lib::Position::bottom:
    cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    lib::add_position_css_class<lib::Position::bottom>(this);
    break;
  }
}

int TimeModule::vin_time_module(lua_State* const L)
{
  if (lua_gettop(L) != 1) {
    return luaL_error(L, "invalid number of arguments, expected 1 but got %d", lua_gettop(L));
  }

  if (!lua_istable(L, 1)) {
    return luaL_error(L, "invalid first argument, expected table but got %s", lua_typename(L, lua_type(L, -1)));
  }

  auto* const module{ static_cast<TimeModule*>(lua_touserdata(L, lua_upvalueindex(1))) };

  lua_getfield(L, 1, "time_format");
  lua_getfield(L, 1, "date_format");

  std::string_view time_format{ module->m_buffer.data(), module->m_date_format };
  std::string_view date_format{ module->m_date_format, module->m_time };

  std::size_t size{};

  if (lua_isstring(L, 2) != 0) {
    const char* data{ lua_tolstring(L, -2, &size) };
    time_format = { data, size };
  }

  if (lua_isstring(L, 3) != 0) {
    const char* data{ lua_tolstring(L, -1, &size) };
    date_format = { data, size };
  }

  module->update_format(time_format, date_format);
  module->update_time();
  module->m_time_label->set_label(module->m_time);
  module->m_date_label->set_label(module->m_date);

  g_assert(lua_gettop(L) == 3);
  lua_pop(L, 3);

  return 0;
}

void TimeModule::vfunc_configure(lua_State* const L)
{
  lua_pushlightuserdata(L, this); // [vin, this]
  lua_pushcclosure(L, vin_time_module, 1); // [vin, vin_time_module]
  lua_setfield(L, 1, "time_module"); // [vin]
}
