#include "vin/lib/iconfigurable.hpp"
#include "vin/lib/ipositionable.hpp"
#include "vin/lib/lua.hpp"
#include "vin/window.hpp"

#include <peel/class.h>
#include <peel/Gio/Initable.h>
#include <peel/Gtk/Align.h>
#include <peel/Gtk/ApplicationWindow.h>
#include <peel/Gtk/Box.h>
#include <peel/Gtk/CenterBox.h>
#include <peel/Gtk/Orientation.h>
#include <peel/Gtk/Widget.h>
#include <peel/Gtk4LayerShell/Gtk4LayerShell.h>
#include <spdlog/spdlog.h>

#include <utility>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
}

using namespace vin;
using namespace peel;

PEEL_CLASS_IMPL(Window, "VinWindow", Gtk::ApplicationWindow)

void Window::Class::init() {}

void Window::init([[maybe_unused]] Class* const cls)
{
  Gtk4LayerShell::init_for_window(this);
  Gtk4LayerShell::set_layer(this, Gtk4LayerShell::Layer::TOP);
  Gtk4LayerShell::set_keyboard_mode(this, Gtk4LayerShell::KeyboardMode::NONE);

  auto layout{ Gtk::CenterBox::create() };
  m_layout = layout;

  auto left_box{ Gtk::Box::create(Gtk::Orientation::HORIZONTAL, 0) };
  left_box->set_name("left-box");
  left_box->set_valign(Gtk::Align::CENTER);
  m_boxes[std::to_underlying(ModuleLocation::left)] = left_box;

  auto center_box{ Gtk::Box::create(Gtk::Orientation::HORIZONTAL, 0) };
  center_box->set_name("center-box");
  center_box->set_valign(Gtk::Align::CENTER);
  m_boxes[std::to_underlying(ModuleLocation::center)] = center_box;

  auto right_box{ Gtk::Box::create(Gtk::Orientation::HORIZONTAL, 0) };
  right_box->set_name("right-box");
  right_box->set_valign(Gtk::Align::CENTER);
  m_boxes[std::to_underlying(ModuleLocation::right)] = right_box;

  layout->set_start_widget(std::move(left_box));
  layout->set_center_widget(std::move(center_box));
  layout->set_end_widget(std::move(right_box));

  set_child(std::move(layout));
}

namespace {

template<lib::Position P>
void position_modules(const auto& modules)
{
  for (const auto& module : modules) {
    if (module.widget != nullptr) {
      module.widget->template cast<lib::IPositionable>()->position(P);
    }
  }
}

template<lib::Position P>
void position_boxes(const auto& boxes)
{
  for (auto* const box : boxes) {
    if constexpr (P == lib::Position::top || P == lib::Position::bottom) {
      box->template cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    } else {
      box->template cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    }
    lib::add_position_css_class<lib::Position::top>(box);
  }
}

} // namespace

void Window::vfunc_position(const lib::Position position)
{
  spdlog::info("position window");
  switch (position) {
  case lib::Position::top:
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::TOP, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::LEFT, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::RIGHT, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::BOTTOM, false);
    m_layout->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    position_boxes<lib::Position::top>(m_boxes);
    position_modules<lib::Position::top>(m_modules);
    lib::add_position_css_class<lib::Position::top>(this);
    break;
  case lib::Position::left:
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::TOP, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::LEFT, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::RIGHT, false);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::BOTTOM, true);
    m_layout->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    position_boxes<lib::Position::left>(m_boxes);
    position_modules<lib::Position::left>(m_modules);
    lib::add_position_css_class<lib::Position::left>(this);
    break;
  case lib::Position::right:
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::TOP, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::LEFT, false);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::RIGHT, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::BOTTOM, true);
    m_layout->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    position_boxes<lib::Position::right>(m_boxes);
    position_modules<lib::Position::right>(m_modules);
    lib::add_position_css_class<lib::Position::right>(this);
    break;
  case lib::Position::bottom:
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::TOP, false);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::LEFT, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::RIGHT, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::BOTTOM, true);
    m_layout->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    position_boxes<lib::Position::bottom>(m_boxes);
    position_modules<lib::Position::bottom>(m_modules);
    lib::add_position_css_class<lib::Position::bottom>(this);
    break;
  }
}

int Window::vin_window(lua_State* const L)
{
  VIN_LUA_EXPECT_NARGS(1);
  VIN_LUA_EXPECT_TABLE(1);

  auto* const window{ static_cast<Window*>(lua_touserdata(L, lua_upvalueindex(1))) };

  spdlog::info("vin window");

  lua_getfield(L, 1, "modules");
  if (lua_istable(L, 2) != 0) {
    create_modules<ModuleLocation::left>(L, window);
    create_modules<ModuleLocation::center>(L, window);
    create_modules<ModuleLocation::right>(L, window);
  }
  lua_pop(L, 1);

  lua_getfield(L, 1, "position");
  if (lua_isinteger(L, 2) != 0) {
    window->vfunc_position(static_cast<lib::Position>(lua_tointeger(L, 2)));
  }
  lua_pop(L, 1);

  lua_getfield(L, 1, "auto_exclusive");
  if (lua_isboolean(L, 2) != 0) {
    if (lua_toboolean(L, 2) != 0) {
      Gtk4LayerShell::auto_exclusive_zone_enable(window);
    } else {
      Gtk4LayerShell::set_exclusive_zone(window, 0);
    }
  }
  lua_pop(L, 1);

  lua_getfield(L, 1, "margin");
  if (lua_istable(L, 2) != 0) {
    lua_getfield(L, 2, "top");
    lua_getfield(L, 2, "left");
    lua_getfield(L, 2, "right");
    lua_getfield(L, 2, "bottom");
    if (lua_isinteger(L, 3) != 0) {
      Gtk4LayerShell::set_margin(window, Gtk4LayerShell::Edge::TOP, lua_tointeger(L, 3));
    }
    if (lua_isinteger(L, 4) != 0) {
      Gtk4LayerShell::set_margin(window, Gtk4LayerShell::Edge::LEFT, lua_tointeger(L, 4));
    }
    if (lua_isinteger(L, 5) != 0) {
      Gtk4LayerShell::set_margin(window, Gtk4LayerShell::Edge::RIGHT, lua_tointeger(L, 5));
    }
    if (lua_isinteger(L, 6) != 0) {
      Gtk4LayerShell::set_margin(window, Gtk4LayerShell::Edge::BOTTOM, lua_tointeger(L, 6));
    }
    lua_pop(L, 4);
  }

  VIN_LUA_ASSERT_POP(L, 2);

  return 0;
}

void Window::vfunc_configure(lua_State* const L)
{
  lua_newtable(L);
  lua_pushinteger(L, static_cast<lua_Integer>(lib::Position::top));
  lua_setfield(L, -2, "top");
  lua_pushinteger(L, static_cast<lua_Integer>(lib::Position::left));
  lua_setfield(L, -2, "left");
  lua_pushinteger(L, static_cast<lua_Integer>(lib::Position::right));
  lua_setfield(L, -2, "right");
  lua_pushinteger(L, static_cast<lua_Integer>(lib::Position::bottom));
  lua_setfield(L, -2, "bottom");
  lua_setfield(L, -2, "position");

  lua_newtable(L);
  lua_pushinteger(L, static_cast<lua_Integer>(Module::worksapce));
  lua_setfield(L, -2, "workspace");
  lua_pushinteger(L, static_cast<lua_Integer>(Module::time));
  lua_setfield(L, -2, "time");
  lua_pushinteger(L, static_cast<lua_Integer>(Module::notification));
  lua_setfield(L, -2, "notification");
  lua_setfield(L, -2, "module");

  lua_pushlightuserdata(L, this);
  lua_pushcclosure(L, vin_window, 1);
  lua_setfield(L, -2, "window");
}

void Window::init_type(const peel::Type type)
{
  PEEL_IMPLEMENT_INTERFACE(type, lib::IPositionable);
  PEEL_IMPLEMENT_INTERFACE(type, lib::IConfigurable);
}

void Window::init_interface(lib::IPositionable::Iface* const iface)
{
  iface->override_vfunc_position<Window>();
}

void Window::init_interface(lib::IConfigurable::Iface* const iface)
{
  iface->override_vfunc_configure<Window>();
}
