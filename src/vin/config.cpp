#include "vin/config.hpp"
#include "vin/error.hpp"
#include "vin/lua.hpp"
#include "vin/main_context.hpp"

#include <peel/GObject/Object.h>
#include <peel/Gio/File.h>
#include <peel/Gio/FileMonitor.h>
#include <peel/Gio/FileMonitorEvent.h>
#include <peel/Gio/FileMonitorFlags.h>
#include <peel/Gio/Initable.h>
#include <peel/Gio/Resource.h>
#include <peel/GLib/Error.h>
#include <peel/GLib/functions.h>
#include <peel/GLib/MainContext.h>
#include <peel/UniquePtr.h>
#include <pwd.h>
#include <spdlog/spdlog.h>

#include <array>
#include <cstdlib>

using namespace vin;
using namespace peel;

namespace {
const char* get_home(auto& buffer, UniquePtr<GLib::Error>* const error)
{
  const auto* const home{ std::getenv("HOME") };

  if (home != nullptr) {
    return home;
  }

  passwd pwd;
  passwd* result{ &pwd };
  const int error_code{ getpwuid_r(geteuid(), result, buffer.data(), buffer.size(), &result) };

  if (error_code != 0) {
    GLib::set_error(error,
      s_quark,
      static_cast<int>(Error::failed_to_determine_home),
      "failed to get calling user's home directory : %s",
      strerrordesc_np(error_code));
    return nullptr;
  }

  return pwd.pw_dir;
}
} // namespace

PEEL_CLASS_IMPL(Config, "VinConfig", Object)

Config::ConfigChanged Config::s_config_changed;
Config::CssChanged Config::s_css_changed;

void Config::Class::init()
{
  s_config_changed = ConfigChanged::create("config-changed");
  s_css_changed = CssChanged::create("css-changed");
}

void Config::init_type(const peel::Type type)
{
  PEEL_IMPLEMENT_INTERFACE(type, peel::Gio::Initable);
}

void Config::init_interface(peel::Gio::Initable::Iface* const iface)
{
  iface->override_vfunc_init<Config>();
}

bool Config::vfunc_init([[maybe_unused]] Gio::Cancellable* const cancellable, UniquePtr<GLib::Error>* const error)
{
  const char* const xdg_config_home{ std::getenv("XDG_CONFIG_HOME") };

  std::array<const char*, 5> parts;
  const char** file{};

  std::array<char, 512 - sizeof(parts)> home_buffer;
  const char* home{};

  if (xdg_config_home != nullptr) {
    parts[0] = xdg_config_home;
    parts[1] = "vin";
    file = parts.data() + 2;
    parts[3] = nullptr;
  } else {
    home = get_home(home_buffer, error);
    if (home == nullptr) {
      return false;
    }
    parts[0] = home;
    parts[1] = ".config";
    parts[2] = "vin";
    file = parts.data() + 3;
    parts[4] = nullptr;
  }

  *file = "main.css";
  m_css_file = Gio::File::create_build_filenamev(StrvRef::adopt(parts.data()));
  *file = "config.lua";
  m_config_file = Gio::File::create_build_filenamev(StrvRef::adopt(parts.data()));

  return true;
}

VIN_IMPLEMENT_MAIN_CONTEXT_PROPERTY(Config)

void Config::define_properties(auto& visitor)
{
  VIN_DEFINE_MAIN_CONTEXT_PROPERTY(Config)
}

void Config::on_css_changed([[maybe_unused]] Gio::FileMonitor* const file_monitor,
  [[maybe_unused]] Gio::File* const file,
  [[maybe_unused]] Gio::File* const other_file,
  const Gio::FileMonitor::Event event)
{
  if (event != Gio::FileMonitor::Event::CHANGED) {
    return;
  }
  m_main_context->invoke([this]() {
    s_css_changed.emit(this);
    return G_SOURCE_REMOVE;
  });
}

void Config::on_config_changed([[maybe_unused]] Gio::FileMonitor* const file_monitor,
  [[maybe_unused]] Gio::File* const file,
  [[maybe_unused]] Gio::File* const other_file,
  const Gio::FileMonitor::Event event)
{
  if (event != Gio::FileMonitor::Event::CHANGED) {
    return;
  }
  m_main_context->invoke([this]() {
    s_config_changed.emit(this);
    return G_SOURCE_REMOVE;
  });
}

void Config::monitor_config([[maybe_unused]] Gio::Cancellable* const cancellable, UniquePtr<GLib::Error>* const error)
{
  m_config_file_monitor = m_config_file->monitor_file(Gio::File::MonitorFlags::NONE, nullptr, error);

  if (!m_config_file_monitor) {
    GLib::set_error(error,
      s_quark,
      static_cast<int>(Error::config_monitor_failed),
      "failed to register file monitor for %s",
      m_css_file->get_path().c_str());
    return;
  }

  m_config_file_monitor->connect_changed(this, &Config::on_config_changed);
}

void Config::monitor_css([[maybe_unused]] Gio::Cancellable* const cancellable, UniquePtr<GLib::Error>* const error)
{
  m_css_file_monitor = m_css_file->monitor_file(Gio::File::MonitorFlags::NONE, nullptr, error);

  if (!m_css_file_monitor) {
    GLib::set_error(error,
      s_quark,
      static_cast<int>(Error::css_monitor_failed),
      "failed to register file monitor for %s",
      m_css_file->get_path().c_str());
    return;
  }

  m_css_file_monitor->connect_changed(this, &Config::on_css_changed);
}

void Config::init_state()
{
  if (m_lua != nullptr) {
    lua_close(m_lua);
  }
  m_lua = luaL_newstate();
  lua_newtable(m_lua);
  lua_pushvalue(m_lua, 1);
  lua_setfield(m_lua, LUA_REGISTRYINDEX, "vin");
}

void Config::finish_state()
{
  lua_setglobal(m_lua, "vin");
}

bool Config::load_config(UniquePtr<GLib::Error>* const error)
{
  g_return_val_if_fail(lua_gettop(m_lua) == 0, false);

  luaL_openselectedlibs(m_lua, LUA_GLIBK | LUA_STRLIBK | LUA_TABLIBK, 0);

  // the lua_pcall message handler
  lua_pushcfunction(m_lua, [](lua_State* const L) -> int {
    luaL_traceback(L, L, lua_tostring(L, 1), 1);
    return 1;
  }); // [handler]

  // load the config as a function
  if (luaL_loadfile(m_lua, m_config_file->get_path().c_str()) != LUA_OK) {
    GLib::set_error(error,
      s_quark,
      static_cast<int>(Error::failed_to_load_config),
      "failed to load config : %s",
      lua_tostring(m_lua, -1));
    VIN_LUA_ASSERT_POP(m_lua, 2);
    return false;
  } // [handler, config_function]

  const auto timeout{ GLib::timeout_source_new(1500) };

  timeout->set_callback([]() {
    spdlog::critical("config timed out after 1.5 seconds"); // TODO: throw on critical logs
    throw std::runtime_error("config timed out");
    return G_SOURCE_REMOVE;
  });

  // timeout->attach(m_worker_context);

  // call the config with the message handler at stack index 1
  if (lua_pcall(m_lua, 0, 0, 1) != LUA_OK) {
    GLib::set_error(error, s_quark, static_cast<int>(Error::failed_to_run_config), lua_tostring(m_lua, -1));
    timeout->destroy();
    VIN_LUA_ASSERT_POP(m_lua, 2);
    return false;
  } // [handler]

  timeout->destroy();

  VIN_LUA_ASSERT_POP(m_lua, 1);

  return true;
}
