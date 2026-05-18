#include "vin/hyprland/event_listener.hpp"
#include "vin/hyprland/workspaces.hpp"
#include "vin/vin.hpp"

#include <fmt/base.h>
#include <fmt/format.h>
#include <peel/class.h>
#include <peel/Gio/ActionGroup.h>
#include <peel/Gio/SimpleAction.h>
#include <peel/GLib/MainLoop.h>
#include <peel/GLib/OptionArg.h>
#include <peel/GLib/OptionEntry.h>
#include <peel/GLib/OptionFlags.h>
#include <peel/GLib/Thread.h>
#include <peel/GLib/Variant.h>
#include <peel/Gtk/Application.h>
#include <peel/Gtk/ApplicationWindow.h>
#include <peel/Gtk/Box.h>
#include <peel/Gtk/CenterBox.h>
#include <peel/Gtk/CssProvider.h>
#include <peel/Gtk/functions.h>
#include <peel/Gtk/Orientation.h>
#include <peel/Gtk4LayerShell/functions.h>
#include <peel/Gtk4LayerShell/Layer.h>
#include <peel/Strv.h>
#include <peel/ZTArrayRef.h>
#include <spdlog/spdlog.h>

#include <array>

using namespace vin;
using namespace peel;

PEEL_CLASS_IMPL(Vin, "VinVin", Gtk::Application)

void Vin::init(Class* const /*cls*/)
{
  spdlog::info("vin instance init");

  connect_startup(this, &Vin::on_startup);
  connect_activate(this, &Vin::on_activate);
  connect_shutdown(this, &Vin::on_shutdown);
  connect_handle_local_options(this, &Vin::on_handle_local_options);

  std::array<GLib::OptionEntry, 4> options{
    GLib::OptionEntry{
      .long_name = "toggle",
      .short_name = 't',
      .flags = static_cast<int>(GLib::OptionFlags::NONE),
      .arg = GLib::OptionArg::NONE,
      .arg_data = &m_arg_toggle,
      .description = "Toggle visibility",
      .arg_description = nullptr,
    },
    GLib::OptionEntry{
      .long_name = "quit",
      .short_name = 'q',
      .flags = static_cast<int>(GLib::OptionFlags::NONE),
      .arg = GLib::OptionArg::NONE,
      .arg_data = &m_arg_quit,
      .description = "Quit vin",
      .arg_description = nullptr,
    },
    GLib::OptionEntry{
      .long_name = "version",
      .short_name = 'v',
      .flags = static_cast<int>(GLib::OptionFlags::NONE),
      .arg = GLib::OptionArg::NONE,
      .arg_data = &m_arg_version,
      .description = "Print version and exit",
      .arg_description = nullptr,
    },
  };

  add_main_option_entries(peel::ZTArrayRef<GLib::OptionEntry>::adopt(options.data()));
}

void Vin::Class::init()
{
  override_vfunc_local_command_line<Vin>();
}

void Vin::on_startup(Gio::Application* const /*app*/)
{
  spdlog::set_level(spdlog::level::trace);

  spdlog::info("vin startup");

  const auto toggle_action{ Gio::SimpleAction::create("toggle-action", nullptr) };
  toggle_action->connect_activate(this, &Vin::on_toggle);
  add_action(toggle_action);

  const auto quit_action{ Gio::SimpleAction::create("quit-action", nullptr) };
  quit_action->connect_activate(this, &Vin::on_quit);
  add_action(quit_action);

  m_main_context = GLib::MainContext::default_();
  m_worker_context = GLib::MainContext::create();

  auto* const display{ Gdk::Display::get_default() };

  spdlog::info("display {}", static_cast<const void*>(display));
  spdlog::info("main context {}", static_cast<const void*>(m_main_context));
  spdlog::info("worker context {}", static_cast<const void*>(m_worker_context));

  GLib::Thread::create("worker_thread", [this]() {
    m_worker_context->push_thread_default();
    const auto worker_loop{ GLib::MainLoop::create(m_worker_context, false) };
    worker_loop->run();
    m_worker_context->pop_thread_default();
    return nullptr;
  });

  m_css_provider = Gtk::CssProvider::create();
  m_css_provider->load_from_path("main.css");
  Gtk::StyleContext::add_provider_for_display(display, m_css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);

  UniquePtr<GLib::Error> error;

  m_hyprland_event_listener = vin::hyprland::EventListener::create(nullptr, &error, m_main_context, m_worker_context);

  if (error) {
    spdlog::error("{}", error->message);
    m_exit_status = 1;
    return;
  }
}

void Vin::on_activate(Gio::Application* const app)
{
  spdlog::info("vin activate");

  if (m_exit_status > 0 || get_active_window() != nullptr) {
    return;
  }

  auto* const window{ Gtk::ApplicationWindow::create(app->cast<Gtk::Application>()) };

  Gtk4LayerShell::init_for_window(window);
  Gtk4LayerShell::set_layer(window, Gtk4LayerShell::Layer::TOP);
  Gtk4LayerShell::auto_exclusive_zone_enable(window);

  Gtk4LayerShell::set_anchor(window, Gtk4LayerShell::Edge::TOP, true);
  Gtk4LayerShell::set_anchor(window, Gtk4LayerShell::Edge::LEFT, true);
  Gtk4LayerShell::set_anchor(window, Gtk4LayerShell::Edge::RIGHT, true);

  auto layout{ Gtk::CenterBox::create() };

  auto left_box{ Gtk::Box::create(Gtk::Orientation::HORIZONTAL, 8) };
  auto center_box{ Gtk::Box::create(Gtk::Orientation::HORIZONTAL, 8) };
  auto right_box{ Gtk::Box::create(Gtk::Orientation::HORIZONTAL, 8) };

  left_box->add_css_class("left-box");
  center_box->add_css_class("center-box");
  right_box->add_css_class("right-box");

  auto workspaces{ hyprland::Workspaces::create(Gtk::Orientation::HORIZONTAL, 8) };

  center_box->append(std::move(workspaces));

  layout->set_start_widget(std::move(left_box));
  layout->set_center_widget(std::move(center_box));
  layout->set_end_widget(std::move(right_box));

  window->set_child(std::move(layout));

  window->present();
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void Vin::on_shutdown(Gio::Application* const /*app*/)
{
  spdlog::info("vin shutdown");
}

// NOLINTNEXTLINE(readability-make-member-function-const)
int Vin::on_handle_local_options(peel::Gio::Application* /*app*/, peel::GLib::VariantDict* /*dict*/)
{
  spdlog::info("vin handle local options");

  if (m_arg_version) {
    fmt::println("0.0.0 (TODO)");
    return 0;
  }

  return -1;
}

bool Vin::vfunc_local_command_line(peel::Strv* const args, int* const exit_status)
{
  spdlog::info("vin local command line");

  const bool handled_all{ parent_vfunc_local_command_line<Vin>(args, exit_status) };

  spdlog::info("handled all {}", handled_all);

  if (m_exit_status > 0) {
    *exit_status = m_exit_status;
    return true;
  }

  if (m_arg_quit) {
    reinterpret_cast<Gio::ActionGroup*>(this)->activate_action("quit-action", nullptr);
  } else if (m_arg_toggle) {
    reinterpret_cast<Gio::ActionGroup*>(this)->activate_action("toggle-action", nullptr);
  }

  return handled_all;
}

void Vin::on_toggle(Gio::SimpleAction* /*action*/, GLib::Variant* /*variant*/)
{
  auto* window{ get_active_window() };
  if (window != nullptr) {
    window->set_visible(!window->get_visible());
  } else {
    spdlog::error("no window to toggle");
  }
}

void Vin::on_quit(Gio::SimpleAction* /*action*/, GLib::Variant* /*variant*/)
{
  quit();
}
