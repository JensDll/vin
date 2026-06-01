#pragma once

#include "vin/config.hpp"
#include "vin/lib/hyprland_event_listener.hpp"

#include <peel/class.h>
#include <peel/Gio/ApplicationFlags.h>
#include <peel/Gio/SimpleAction.h>
#include <peel/GLib/MainContext.h>
#include <peel/GLib/VariantDict.h>
#include <peel/Gtk/Application.h>
#include <peel/Gtk/CssProvider.h>
#include <peel/RefPtr.h>
#include <peel/Strv.h>

namespace vin {

class Application final : public peel::Gtk::Application
{
private:
  PEEL_SIMPLE_CLASS(Application, peel::Gtk::Application)

  friend class peel::Gio::Application;

  peel::RefPtr<Config> m_config;
  peel::RefPtr<lib::HyprlandEventListener> m_hyprland_event_listener;
  peel::GLib::MainContext* m_main_context;
  peel::RefPtr<peel::GLib::MainContext> m_worker_context;
  peel::RefPtr<peel::Gtk::CssProvider> m_css_provider;
  int m_exit_status;
  bool m_arg_toggle;
  bool m_arg_quit;
  bool m_arg_version;

public:
  ~Application() = default;

  static auto create()
  {
    return peel::Object::create<Application>(
      prop_application_id(), "com.doellmann.vin", prop_flags(), peel::Gio::Application::Flags::DEFAULT_FLAGS);
  }

private:
  void init(Class* cls);

  void on_toggle(peel::Gio::SimpleAction* action, peel::GLib::Variant* variant);

  void on_quit(peel::Gio::SimpleAction* action, peel::GLib::Variant* variant);

  void on_startup(peel::Gio::Application* app);

  void on_activate(peel::Gio::Application* app);

  void on_shutdown(peel::Gio::Application* app);

  int on_handle_local_options(peel::Gio::Application* app, peel::GLib::VariantDict* dict);

  void on_config_changed(Config* config);

  void on_css_changed(Config* config);

  bool vfunc_local_command_line(peel::Strv* args, int* exit_status);

  void start_worker_thread();
};

} // namespace vin
