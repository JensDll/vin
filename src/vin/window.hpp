#pragma once

#include "vin/application.hpp"
#include "vin/lib/iconfigurable.hpp"
#include "vin/lib/ipositionable.hpp"
#include "vin/module/time.hpp"
#include "vin/module/workspace.hpp"

#include <fmt/format.h>
#include <peel/class.h>
#include <peel/FloatPtr.h>
#include <peel/GLib/Error.h>
#include <peel/GLib/MainContext.h>
#include <peel/Gtk/ApplicationWindow.h>
#include <peel/Gtk/Box.h>
#include <peel/Gtk/CenterBox.h>
#include <peel/Gtk/Widget.h>
#include <peel/property.h>
#include <peel/RefPtr.h>
#include <peel/UniquePtr.h>
#include <spdlog/spdlog.h>

#include <array>
#include <cstddef>
#include <utility>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
}

namespace vin {

class Window final : public peel::Gtk::ApplicationWindow
{
  PEEL_SIMPLE_CLASS(Window, peel::Gtk::ApplicationWindow)

  friend class lib::IPositionable;
  friend class lib::IConfigurable;
  friend class peel::Gio::Initable;

  enum class Module : unsigned char { worksapce, time, notification, NUM };

  enum class ModuleLocation : unsigned char { left, center, right };

  struct ModuleEntry
  {
    peel::Gtk::Widget* widget;
    std::size_t position;
    ModuleLocation location;
  };

  std::array<ModuleEntry, std::to_underlying(Module::NUM)> m_modules;

  peel::Gtk::CenterBox* m_layout;

  std::array<peel::Gtk::Box*, 3> m_boxes;

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

  void test_remove()
  {
    spdlog::info("remove");
    m_boxes[std::to_underlying(ModuleLocation::center)]->remove(
      m_modules[std::to_underlying(Module::worksapce)].widget);
  }

private:
  void init(Class* cls);

  static void init_type(peel::Type type);

  static void init_interface(lib::IPositionable::Iface* iface);

  static void init_interface(lib::IConfigurable::Iface* iface);

  void vfunc_configure(lua_State* L);

  void vfunc_position(lib::Position position);

  template<ModuleLocation Loc>
  static void configure_modules(lua_State* const L, Window* const window)
  {
    if constexpr (Loc == ModuleLocation::left) {
      lua_getfield(L, -1, "left");
    } else if constexpr (Loc == ModuleLocation::center) {
      lua_getfield(L, -1, "center");
    } else if constexpr (Loc == ModuleLocation::right) {
      lua_getfield(L, -1, "right");
    }

    if (lua_istable(L, -1) == 0) {
      luaL_error(L, "expected array of modules");
      return;
    }

    lua_pushnil(L);

    peel::UniquePtr<peel::GLib::Error> error;

    auto to_remove{ window->m_modules };

    for (std::size_t i{}; lua_next(L, -2) != 0; ++i) {
      if (lua_isnumber(L, -2) == 0 || lua_isnumber(L, -1) == 0) {
        luaL_error(L, "expected array of modules");
        return;
      }

      switch (Module(lua_tointeger(L, -1))) {
      case Module::time:
        add_module<Module::time, Loc>(L, &error, i, window);
        to_remove[std::to_underlying(Module::time)].widget = nullptr;
        break;
      case Module::worksapce:
        add_module<Module::worksapce, Loc>(L, &error, i, window);
        to_remove[std::to_underlying(Module::worksapce)].widget = nullptr;
        break;
      case Module::notification:
        add_module<Module::notification, Loc>(L, &error, i, window);
        to_remove[std::to_underlying(Module::worksapce)].widget = nullptr;
        break;
      case Module::NUM:
        break;
      }

      if (error) {
        luaL_error(L, "%s", error->message);
        return;
      }

      lua_pop(L, 1);
    }

    lua_getfield(L, LUA_REGISTRYINDEX, "vin");

    for (std::size_t i{}; i < std::to_underlying(Module::NUM); ++i) {
      const auto& entry{ to_remove[i] };
      if (entry.widget != nullptr && entry.location == Loc) {
        window->m_modules[i] = {};
        entry.widget->cast<lib::IConfigurable>()->unconfigure(L);
        window->m_boxes[std::to_underlying(Loc)]->remove(entry.widget);
      }
    }

    lua_pop(L, 2);
  }

  template<Module M, ModuleLocation Loc>
  static void add_module(lua_State* const L,
    peel::UniquePtr<peel::GLib::Error>* const error,
    const std::size_t position,
    Window* const window)
  {
    auto& entry{ window->m_modules[std::to_underlying(M)] };

    if (entry.widget != nullptr) {
      if (entry.position == position && entry.location == Loc) {
        return;
      }
      window->m_boxes[std::to_underlying(entry.location)]->remove(entry.widget);
    }

    peel::RefPtr<peel::Gtk::Widget> module;

    if constexpr (M == Module::worksapce) {
      module = module::Workspace::create(nullptr, error, window->m_main_context, window->m_worker_context);
      if (*error) {
        return;
      }
    } else if constexpr (M == Module::time) {
      module = module::Time::create(window->m_main_context, window->m_worker_context);
    }

    entry = { .widget = module, .position = position, .location = Loc };

    lua_getfield(L, LUA_REGISTRYINDEX, "vin");
    module->cast<lib::IConfigurable>()->configure(L);
    lua_pop(L, 1);

    window->m_boxes[std::to_underlying(Loc)]->append(module);
  }

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
