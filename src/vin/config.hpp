#pragma once

#include "vin/main_context.hpp"

#include <peel/GObject/Object.h>
#include <peel/GObject/ParamFlags.h>
#include <peel/ArrayRef.h>
#include <peel/class.h>
#include <peel/Gio/Cancellable.h>
#include <peel/Gio/File.h>
#include <peel/Gio/FileMonitor.h>
#include <peel/Gio/Initable.h>
#include <peel/Gio/Settings.h>
#include <peel/GLib/Error.h>
#include <peel/GLib/MainContext.h>
#include <peel/RefPtr.h>
#include <peel/signal.h>
#include <spdlog/spdlog.h>

extern "C" {
#include <lua.h>
}

namespace vin {

class Config final : public peel::Object
{
  PEEL_SIMPLE_CLASS(Config, peel::Gio::Initable)

  friend class peel::Gio::Initable;

  using CssChanged = peel::Signal<Config, void()>;
  using ConfigChanged = peel::Signal<Config, void()>;

  static ConfigChanged s_config_changed;
  static CssChanged s_css_changed;

  peel::RefPtr<peel::Gio::File> m_config_file;
  peel::RefPtr<peel::Gio::File> m_css_file;

  peel::RefPtr<peel::Gio::FileMonitor> m_config_file_monitor;
  peel::RefPtr<peel::Gio::FileMonitor> m_css_file_monitor;

  peel::GLib::MainContext* m_main_context;
  peel::GLib::MainContext* m_worker_context;

  lua_State* m_lua;

public:
  ~Config() = default;

  static auto create(const MainContext context)
  {
    return peel::Object::create<Config>(
      prop_main_context(), context.main_context, prop_worker_context(), context.worker_context);
  }

  [[nodiscard]] peel::Gio::File* get_config_file() const
  {
    return m_config_file;
  }

  [[nodiscard]] peel::Gio::File* get_css_file() const
  {
    return m_css_file;
  }

  [[nodiscard]] lua_State* get_state() const
  {
    return m_lua;
  }

  void monitor_config(peel::Gio::Cancellable* cancellable, peel::UniquePtr<peel::GLib::Error>* error);

  void monitor_css(peel::Gio::Cancellable* cancellable, peel::UniquePtr<peel::GLib::Error>* error);

  void init_state();

  void finish_state();

  bool load_config(peel::UniquePtr<peel::GLib::Error>* error);

  VIN_MAIN_CONTEXT_PROPERTY

  PEEL_SIGNAL_CONNECT_METHOD(config_changed, s_config_changed)
  PEEL_SIGNAL_CONNECT_METHOD(css_changed, s_css_changed)

private:
  static void init_type(peel::Type type);

  static void init_interface(peel::Gio::Initable::Iface* iface);

  bool vfunc_init(peel::Gio::Cancellable* cancellable, peel::UniquePtr<peel::GLib::Error>* error);

  void on_css_changed(peel::Gio::FileMonitor* file_monitor,
    peel::Gio::File* file,
    peel::Gio::File* other_file,
    peel::Gio::FileMonitor::Event event);

  void on_config_changed(peel::Gio::FileMonitor* file_monitor,
    peel::Gio::File* file,
    peel::Gio::File* other_file,
    peel::Gio::FileMonitor::Event event);

  static void define_properties(auto& visitor);
};

} // namespace vin
