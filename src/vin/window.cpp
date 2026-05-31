#include "vin/lib/iconfigurable.hpp"
#include "vin/lib/ipositionable.hpp"
#include "vin/module/time_module.hpp"
#include "vin/module/workspace_module.hpp"
#include "vin/window.hpp"

#include <peel/class.h>
#include <peel/Gtk/ApplicationWindow.h>
#include <peel/Gtk/CenterBox.h>
#include <peel/Gtk/Orientation.h>
#include <peel/Gtk/Widget.h>
#include <peel/Gtk4LayerShell/Gtk4LayerShell.h>
#include <spdlog/spdlog.h>

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
  m_left_box = left_box;

  auto center_box{ Gtk::Box::create(Gtk::Orientation::HORIZONTAL, 0) };
  center_box->set_name("center-box");
  m_center_box = center_box;

  auto right_box{ Gtk::Box::create(Gtk::Orientation::HORIZONTAL, 0) };
  right_box->set_name("right-box");
  m_right_box = right_box;

  auto workspace_module{ module::WorkspaceModule::create() };
  m_center_modules.push_back(workspace_module);
  center_box->append(std::move(workspace_module));

  auto time_module{ module::TimeModule::create(m_main_context, m_worker_context) };
  m_right_modules.push_back(time_module);
  right_box->append(std::move(time_module));

  vfunc_position(lib::Position::bottom);

  layout->set_start_widget(std::move(left_box));
  layout->set_center_widget(std::move(center_box));
  layout->set_end_widget(std::move(right_box));

  set_child(std::move(layout));
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

namespace {

template<lib::Position Position>
void position_widgets(const std::span<Gtk::Widget*> widgets)
{
  for (auto* const widget : widgets) {
    widget->cast<lib::IPositionable>()->position(Position);
  }
}

void configure_widgets(const std::span<Gtk::Widget*> widgets, lua_State* const L)
{
  for (auto* const widget : widgets) {
    widget->cast<lib::IConfigurable>()->configure(L);
  }
}

template<Gtk::Orientation Orientation>
void position_box(Gtk::Widget* const box, const char* const css_class)
{
  box->cast<Gtk::Orientable>()->set_orientation(Orientation);
  box->remove_css_class("top");
  box->remove_css_class("left");
  box->remove_css_class("right");
  box->remove_css_class("bottom");
  box->add_css_class(css_class);
}

} // namespace

void Window::vfunc_position(const lib::Position position)
{
  switch (position) {
  case lib::Position::top:
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::TOP, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::LEFT, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::RIGHT, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::BOTTOM, false);
    m_layout->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    position_box<Gtk::Orientation::HORIZONTAL>(m_left_box, "top");
    position_box<Gtk::Orientation::HORIZONTAL>(m_center_box, "top");
    position_box<Gtk::Orientation::HORIZONTAL>(m_right_box, "top");
    position_widgets<lib::Position::top>(m_left_modules);
    position_widgets<lib::Position::top>(m_center_modules);
    position_widgets<lib::Position::top>(m_right_modules);
    break;
  case lib::Position::left:
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::TOP, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::LEFT, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::RIGHT, false);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::BOTTOM, true);
    m_layout->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    position_box<Gtk::Orientation::VERTICAL>(m_left_box, "left");
    position_box<Gtk::Orientation::VERTICAL>(m_center_box, "left");
    position_box<Gtk::Orientation::VERTICAL>(m_right_box, "left");
    position_widgets<lib::Position::left>(m_left_modules);
    position_widgets<lib::Position::left>(m_center_modules);
    position_widgets<lib::Position::left>(m_right_modules);
    break;
  case lib::Position::right:
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::TOP, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::LEFT, false);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::RIGHT, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::BOTTOM, true);
    m_layout->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    position_box<Gtk::Orientation::VERTICAL>(m_left_box, "right");
    position_box<Gtk::Orientation::VERTICAL>(m_center_box, "right");
    position_box<Gtk::Orientation::VERTICAL>(m_right_box, "right");
    position_widgets<lib::Position::right>(m_left_modules);
    position_widgets<lib::Position::right>(m_center_modules);
    position_widgets<lib::Position::right>(m_right_modules);
    break;
  case lib::Position::bottom:
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::TOP, false);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::LEFT, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::RIGHT, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::BOTTOM, true);
    m_layout->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    position_box<Gtk::Orientation::HORIZONTAL>(m_left_box, "bottom");
    position_box<Gtk::Orientation::HORIZONTAL>(m_center_box, "bottom");
    position_box<Gtk::Orientation::HORIZONTAL>(m_right_box, "bottom");
    position_widgets<lib::Position::bottom>(m_left_modules);
    position_widgets<lib::Position::bottom>(m_center_modules);
    position_widgets<lib::Position::bottom>(m_right_modules);
    break;
  }
}

int Window::vin_window(lua_State* const L)
{
  if (lua_gettop(L) != 1) {
    luaL_error(L, "invalid number of arguments, expected 1 but got %d", lua_gettop(L));
    return 0;
  }

  if (!lua_istable(L, -1)) {
    luaL_error(L, "invalid first argument, expected table but got %s", lua_typename(L, lua_type(L, -1)));
    return 0;
  }

  auto* const window{ static_cast<Window*>(lua_touserdata(L, lua_upvalueindex(1))) };

  lua_getfield(L, -1, "position");

  if (lua_isinteger(L, -1) != 0) {
    window->vfunc_position(static_cast<lib::Position>(lua_tointeger(L, -1)));
  }

  lua_pop(L, 1);

  lua_getfield(L, -1, "auto_exclusive");

  if (lua_isboolean(L, -1) != 0) {
    if (lua_toboolean(L, -1) != 0) {
      Gtk4LayerShell::auto_exclusive_zone_enable(window);
    } else {
      Gtk4LayerShell::set_exclusive_zone(window, 0);
    }
  }

  lua_pop(L, 1);

  return 0;
}

void Window::vfunc_configure(lua_State* const L)
{
  spdlog::info("configure window");
  lua_pushlightuserdata(L, this); // [vin, this]
  lua_pushcclosure(L, vin_window, 1); // [vin, vin_window]
  lua_setfield(L, 1, "window"); // [vin]
  configure_widgets(m_left_modules, L);
  configure_widgets(m_center_modules, L);
  configure_widgets(m_right_modules, L);
}
