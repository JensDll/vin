#include "vin/iconfigurable.hpp"
#include "vin/ipositionable.hpp"
#include "vin/lua.hpp"
#include "vin/main_context.hpp"
#include "vin/window.hpp"

#include <peel/class.h>
#include <peel/Gio/Initable.h>
#include <peel/GLib/MainContext.h>
#include <peel/Gtk/Align.h>
#include <peel/Gtk/ApplicationWindow.h>
#include <peel/Gtk/Box.h>
#include <peel/Gtk/CenterBox.h>
#include <peel/Gtk/Orientable.h>
#include <peel/Gtk/Orientation.h>
#include <peel/Gtk/Widget.h>
#include <peel/Gtk4LayerShell/functions.h>
#include <peel/Gtk4LayerShell/Gtk4LayerShell.h>
#include <peel/widget-template.h>
#include <spdlog/spdlog.h>

#include <utility>

using namespace vin;
using namespace peel;

PEEL_CLASS_IMPL(Window, "VinWindow", Gtk::ApplicationWindow)

void Window::Class::init()
{
  set_css_name("main-window");
  set_template_from_resource("/com/doellmann/vin/window.ui");
  PEEL_WIDGET_TEMPLATE_BIND_CHILD(Window, m_layout, "layout");
  PEEL_WIDGET_TEMPLATE_BIND_CHILD(Window, m_boxes[std::to_underlying(ModuleLocation::start)], "start_box");
  PEEL_WIDGET_TEMPLATE_BIND_CHILD(Window, m_boxes[std::to_underlying(ModuleLocation::center)], "center_box");
  PEEL_WIDGET_TEMPLATE_BIND_CHILD(Window, m_boxes[std::to_underlying(ModuleLocation::end)], "end_box");
  override_vfunc_dispose<Window>();
}

void Window::init([[maybe_unused]] Class* const cls)
{
  init_template();
  Gtk4LayerShell::init_for_window(this);
  Gtk4LayerShell::set_layer(this, Gtk4LayerShell::Layer::TOP);
  Gtk4LayerShell::set_keyboard_mode(this, Gtk4LayerShell::KeyboardMode::NONE);
  Gtk4LayerShell::set_namespace(this, "vin");
}

void Window::init_type(const peel::Type type)
{
  PEEL_IMPLEMENT_INTERFACE(type, IPositionable);
  PEEL_IMPLEMENT_INTERFACE(type, IConfigurable);
}

void Window::init_interface(IPositionable::Iface* const iface)
{
  iface->override_vfunc_position<Window>();
}

void Window::init_interface(IConfigurable::Iface* const iface)
{
  iface->override_vfunc_configure<Window>();
}

void Window::vfunc_dispose()
{
  spdlog::info("[VinWindow] dipose");
  set_child(nullptr);
  dispose_template(Type::of<Window>());
  parent_vfunc_dispose<Window>();
}

VIN_IMPLEMENT_MAIN_CONTEXT_PROPERTY(Window)

void Window::define_properties(auto& visitor)
{
  VIN_DEFINE_MAIN_CONTEXT_PROPERTY(Window)
}

namespace {
template<Position P>
void position_modules(const auto& modules)
{
  for (const auto& module : modules) {
    if (module.widget != nullptr) {
      module.widget->template cast<IPositionable>()->position(P);
    }
  }
}

template<Position P>
void position_boxes(const auto& boxes)
{
  for (auto* const box : boxes) {
    if constexpr (P == Position::top || P == Position::bottom) {
      box->template cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    } else {
      box->template cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    }
    add_position_css_class<Position::top>(box);
  }
}
} // namespace

void Window::vfunc_position(const Position position)
{
  switch (position) {
  case Position::top:
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::TOP, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::LEFT, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::RIGHT, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::BOTTOM, false);
    m_layout->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    position_boxes<Position::top>(m_boxes);
    position_modules<Position::top>(m_modules);
    add_position_css_class<Position::top>(this);
    break;
  case Position::left:
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::TOP, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::LEFT, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::RIGHT, false);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::BOTTOM, true);
    m_layout->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    position_boxes<Position::left>(m_boxes);
    position_modules<Position::left>(m_modules);
    add_position_css_class<Position::left>(this);
    break;
  case Position::right:
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::TOP, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::LEFT, false);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::RIGHT, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::BOTTOM, true);
    m_layout->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::VERTICAL);
    position_boxes<Position::right>(m_boxes);
    position_modules<Position::right>(m_modules);
    add_position_css_class<Position::right>(this);
    break;
  case Position::bottom:
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::TOP, false);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::LEFT, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::RIGHT, true);
    Gtk4LayerShell::set_anchor(this, Gtk4LayerShell::Edge::BOTTOM, true);
    m_layout->cast<Gtk::Orientable>()->set_orientation(Gtk::Orientation::HORIZONTAL);
    position_boxes<Position::bottom>(m_boxes);
    position_modules<Position::bottom>(m_modules);
    add_position_css_class<Position::bottom>(this);
    break;
  }
}

int Window::vin_window(lua_State* const L)
{
  VIN_LUA_EXPECT_NARGS(1);
  VIN_LUA_EXPECT_TABLE(1);

  auto* const window{ static_cast<Window*>(lua_touserdata(L, lua_upvalueindex(1))) };

  lua_getfield(L, 1, "modules");
  if (lua_istable(L, 2) != 0) {
    std::array<bool, std::to_underlying(Module::COUNT)> do_not_remove{};

    configure_modules<ModuleLocation::start>(L, window, do_not_remove);
    configure_modules<ModuleLocation::center>(L, window, do_not_remove);
    configure_modules<ModuleLocation::end>(L, window, do_not_remove);

    lua_getfield(L, LUA_REGISTRYINDEX, "vin");

    for (std::size_t i{}; i < std::to_underlying(Module::COUNT); ++i) {
      auto& module{ window->m_modules[i] };
      if (do_not_remove[i] || module.widget == nullptr) {
        continue;
      }
      spdlog::info("remove module");
      module.widget->cast<IConfigurable>()->unconfigure(L);
      window->m_boxes[std::to_underlying(module.location)]->remove(module.widget);
      module = {};
    }

    lua_pop(L, 1);
  }
  lua_pop(L, 1);

  lua_getfield(L, 1, "position");
  if (lua_isinteger(L, 2) != 0) {
    window->vfunc_position(static_cast<Position>(lua_tointeger(L, 2)));
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
  lua_pushinteger(L, static_cast<lua_Integer>(Position::top));
  lua_setfield(L, -2, "top");
  lua_pushinteger(L, static_cast<lua_Integer>(Position::left));
  lua_setfield(L, -2, "left");
  lua_pushinteger(L, static_cast<lua_Integer>(Position::right));
  lua_setfield(L, -2, "right");
  lua_pushinteger(L, static_cast<lua_Integer>(Position::bottom));
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
