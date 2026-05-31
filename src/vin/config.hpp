#pragma once

#include "vin/peel/gio.hpp"

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

class Config final : public peel::Gio::Initable
{
  PEEL_SIMPLE_CLASS(Config, peel::Gio::Initable)

  friend class peel::Gio::Initable;

  using SignalCssChanged = peel::Signal<Config, void()>;
  using SignalConfigChanged = peel::Signal<Config, void()>;

  static SignalConfigChanged s_signal_config_changed;
  static SignalCssChanged s_signal_css_changed;

  peel::RefPtr<peel::Gio::File> m_config_file;
  peel::RefPtr<peel::Gio::File> m_css_file;

  peel::RefPtr<peel::Gio::FileMonitor> m_config_file_monitor;
  peel::RefPtr<peel::Gio::FileMonitor> m_css_file_monitor;

  peel::GLib::MainContext* m_main_context;
  peel::GLib::MainContext* m_worker_context;

  lua_State* m_lua;

public:
  ~Config() = default;

  static auto create(peel::Gio::Cancellable* const cancellable,
    peel::UniquePtr<::peel::GLib::Error>* const error,
    peel::GLib::MainContext* const main_context,
    peel::GLib::MainContext* const worker_context)
  {
    return vin::gio::initable_create<Config>(
      cancellable, error, prop_main_context(), main_context, prop_worker_context(), worker_context);
  }

  [[nodiscard]] peel::Gio::File* config_file() const
  {
    return m_config_file;
  }

  [[nodiscard]] peel::Gio::File* css_file() const
  {
    return m_css_file;
  }

  [[nodiscard]] lua_State* state() const
  {
    return m_lua;
  }

  void monitor_config(peel::Gio::Cancellable* cancellable, peel::UniquePtr<peel::GLib::Error>* error);

  void monitor_css(peel::Gio::Cancellable* cancellable, peel::UniquePtr<peel::GLib::Error>* error);

  void init_state();

  void finish_init_state();

  bool load_config(peel::UniquePtr<peel::GLib::Error>* error);

  PEEL_SIGNAL_CONNECT_METHOD(config_changed, s_signal_config_changed)

  PEEL_SIGNAL_CONNECT_METHOD(css_changed, s_signal_css_changed)

private:
  void init(Class* cls);

  static void init_type(const peel::Type type)
  {
    PEEL_IMPLEMENT_INTERFACE(type, peel::Gio::Initable);
  }

  static void init_interface(peel::Gio::Initable::Iface* const iface)
  {
    iface->override_vfunc_init<Config>();
  }

  bool vfunc_init(peel::Gio::Cancellable* cancellable, peel::UniquePtr<peel::GLib::Error>* error);

  static peel::RefPtr<peel::Object> vfunc_constructor(peel::Type type,
    peel::ArrayRef<peel::Object::ConstructParam> params);

  void on_css_changed(peel::Gio::FileMonitor* file_monitor,
    peel::Gio::File* file,
    peel::Gio::File* other_file,
    peel::Gio::FileMonitor::Event event);

  void on_config_changed(peel::Gio::FileMonitor* file_monitor,
    peel::Gio::File* file,
    peel::Gio::File* other_file,
    peel::Gio::FileMonitor::Event event);

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
    visitor.prop(prop_main_context()).set(&Config::set_main_context).flags(peel::GObject::ParamFlags::CONSTRUCT_ONLY);
    visitor.prop(prop_worker_context())
      .set(&Config::set_worker_context)
      .flags(peel::GObject::ParamFlags::CONSTRUCT_ONLY);
  }
};

} // namespace vin
