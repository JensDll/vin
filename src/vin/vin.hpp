#pragma once

#include "vin/hyprland/event_listener.hpp"

#include <peel/GObject/Object.h>
#include <peel/class.h>
#include <peel/Gio/Application.h>
#include <peel/Gio/ApplicationCommandLine.h>
#include <peel/Gio/ApplicationFlags.h>
#include <peel/Gio/SimpleAction.h>
#include <peel/GLib/MainContext.h>
#include <peel/GLib/Variant.h>
#include <peel/GLib/VariantDict.h>
#include <peel/Gtk/Application.h>
#include <peel/Gtk/CssProvider.h>
#include <peel/RefPtr.h>
#include <peel/Strv.h>

namespace vin {

class Vin final : public peel::Gtk::Application
{
  PEEL_SIMPLE_CLASS(Vin, peel::Gtk::Application)

  friend class peel::Gio::Application;

  peel::GLib::MainContext* m_main_context;
  peel::RefPtr<peel::GLib::MainContext> m_worker_context;
  peel::RefPtr<peel::Gtk::CssProvider> m_css_provider;
  peel::RefPtr<hyprland::EventListener> m_hyprland_event_listener;
  int m_exit_status;
  bool m_arg_toggle;
  bool m_arg_quit;
  bool m_arg_version;

public:
  ~Vin() = default;

  static auto create()
  {
    return peel::Object::create<Vin>(
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

  bool vfunc_local_command_line(peel::Strv* args, int* exit_status);
};

} // namespace vin
