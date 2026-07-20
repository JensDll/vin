#pragma once

#include "vin/application.hpp"
#include "vin/hyprland_workspace_module.hpp"
#include "vin/iconfigurable.hpp"
#include "vin/ipositionable.hpp"
#include "vin/lua.hpp"
#include "vin/main_context.hpp"
#include "vin/notification_module.hpp"
#include "vin/time_module.hpp"

#include <fmt/format.h>
#include <peel/class.h>
#include <peel/FloatPtr.h>
#include <peel/Gio/Initable.h>
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

namespace vin {

class Window final : public peel::Gtk::ApplicationWindow
{
  PEEL_SIMPLE_CLASS(Window, peel::Gtk::ApplicationWindow)

  friend class IPositionable;
  friend class IConfigurable;

  enum class Module : unsigned char { worksapce, time, notification, COUNT };

  enum class ModuleLocation : unsigned char { start, center, end };

  struct ModuleEntry
  {
    peel::Gtk::Widget* widget;
    std::size_t position;
    ModuleLocation location;
  };

  ModuleEntry m_modules[std::to_underlying(Module::COUNT)];

  peel::Gtk::CenterBox* m_layout;

  peel::Gtk::Box* m_boxes[3];

  peel::GLib::MainContext* m_main_context;
  peel::GLib::MainContext* m_worker_context;

public:
  ~Window() = default;

  static auto create(Application* const app, const MainContext context)
  {
    return peel::Object::create<Window>(prop_application(),
      app,
      prop_main_context(),
      context.main_context,
      prop_worker_context(),
      context.worker_context);
  }

  VIN_MAIN_CONTEXT_PROPERTY

private:
  void init(Class* cls);

  static void init_type(peel::Type type);

  static void init_interface(IPositionable::Iface* iface);

  static void init_interface(IConfigurable::Iface* iface);

  void vfunc_configure(lua_State* L);

  void vfunc_position(Position position);

  void vfunc_dispose();

  static int vin_window(lua_State* L);

  static void define_properties(auto& visitor);

  template<ModuleLocation Loc>
  static void configure_modules(lua_State* const L,
    Window* const window,
    std::array<bool, std::to_underlying(Module::COUNT)>& do_not_remove)
  {
    if constexpr (Loc == ModuleLocation::start) {
      lua_getfield(L, -1, "start");
    } else if constexpr (Loc == ModuleLocation::center) {
      lua_getfield(L, -1, "center");
    } else if constexpr (Loc == ModuleLocation::end) {
      lua_getfield(L, -1, "end_");
    }

    if (lua_isnil(L, -1) != 0) {
      lua_pop(L, 1);
      return;
    }

    if (lua_istable(L, -1) == 0) {
      luaL_error(L, "expected array of modules (not a table)");
      return;
    }

    lua_pushnil(L);

    peel::UniquePtr<peel::GLib::Error> error;

    for (std::size_t i{}; lua_next(L, -2) != 0; ++i) {
      if (lua_isinteger(L, -2) == 0) {
        luaL_error(L, "expected array of modules (key not an integer)");
        return;
      }

      if (lua_isinteger(L, -1) == 0) {
        luaL_error(L, "expected array of modules (value not an integer)");
        return;
      }

      switch (Module(lua_tointeger(L, -1))) {
      case Module::time:
        configure_module<Module::time, Loc>(L, &error, i, window);
        do_not_remove[std::to_underlying(Module::time)] = true;
        break;
      case Module::worksapce:
        configure_module<Module::worksapce, Loc>(L, &error, i, window);
        do_not_remove[std::to_underlying(Module::worksapce)] = true;
        break;
      case Module::notification:
        configure_module<Module::notification, Loc>(L, &error, i, window);
        do_not_remove[std::to_underlying(Module::notification)] = true;
        break;
      case Module::COUNT:
        break;
      }

      if (error) {
        luaL_error(L, "%s", error->message);
        return;
      }

      lua_pop(L, 1);
    }

    lua_pop(L, 1);
  }

  template<Module M, ModuleLocation Loc>
  static void configure_module(lua_State* const L,
    peel::UniquePtr<peel::GLib::Error>* const error,
    const std::size_t position,
    Window* const window)
  {
    auto& module{ window->m_modules[std::to_underlying(M)] };

    peel::RefPtr<peel::Gtk::Widget> widget;

    if (module.widget == nullptr) {
      const MainContext context{ .main_context = window->m_main_context, .worker_context = window->m_worker_context };

      if constexpr (M == Module::worksapce) {
        widget = HyprlandWorkspaceModule::create(context);
      } else if constexpr (M == Module::time) {
        widget = TimeModule::create(context);
      } else if constexpr (M == Module::notification) {
        widget = NotificationModule::create(context);
      }

      if (!widget->cast<peel::Gio::Initable>()->init(nullptr, error)) {
        return;
      }
    } else {
      if (module.position == position && module.location == Loc) {
        return;
      }
      widget = module.widget;
      window->m_boxes[std::to_underlying(module.location)]->remove(module.widget);
    }

    module = { .widget = widget, .position = position, .location = Loc };

    lua_getfield(L, LUA_REGISTRYINDEX, "vin");
    widget->cast<IConfigurable>()->configure(L);
    lua_pop(L, 1);

    window->m_boxes[std::to_underlying(Loc)]->append(widget);
  }
};

} // namespace vin
