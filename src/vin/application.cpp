#include "vin/application.hpp"
#include "vin/config.hpp"
#include "vin/lib/iconfigurable.hpp"
#include "vin/window.hpp"

#include <fmt/format.h>
#include <peel/Gdk/Display.h>
#include <peel/Gio/ActionGroup.h>
#include <peel/Gio/SimpleAction.h>
#include <peel/GLib/Error.h>
#include <peel/GLib/MainContext.h>
#include <peel/GLib/MainLoop.h>
#include <peel/GLib/OptionArg.h>
#include <peel/GLib/OptionEntry.h>
#include <peel/GLib/OptionFlags.h>
#include <peel/GLib/Thread.h>
#include <peel/GLib/VariantDict.h>
#include <peel/Gtk/Application.h>
#include <peel/Gtk/StyleContext.h>
#include <peel/Gtk/Window.h>
#include <peel/RefPtr.h>
#include <peel/UniquePtr.h>
#include <peel/ZTArrayRef.h>
#include <spdlog/spdlog.h>

#include <array>

using namespace vin;
using namespace peel;

PEEL_CLASS_IMPL(Application, "VinApplication", Gtk::Application)

void Application::init([[maybe_unused]] Class* const cls)
{
  connect_startup(this, &Application::on_startup);
  connect_activate(this, &Application::on_activate);
  connect_shutdown(this, &Application::on_shutdown);
  connect_handle_local_options(this, &Application::on_handle_local_options);

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

  add_main_option_entries(ZTArrayRef<GLib::OptionEntry>::adopt(options.data()));
}

void Application::Class::init()
{
  override_vfunc_local_command_line<Application>();
}

void Application::on_startup([[maybe_unused]] Gio::Application* const app)
{
  spdlog::set_level(spdlog::level::trace);

  spdlog::info("vin startup");

  m_main_context = GLib::MainContext::default_();
  m_worker_context = GLib::MainContext::create();

  UniquePtr<GLib::Error> error;

  m_config = Config::create(nullptr, &error, m_main_context, m_worker_context);

  if (error) {
    spdlog::error("{}", error->message);
    m_exit_status = 1;
    return;
  }

  m_config->connect_css_changed(this, &Application::on_css_changed);
  m_config->connect_config_changed(this, &Application::on_config_changed);

  start_worker_thread();

  const auto toggle_action{ Gio::SimpleAction::create("toggle-action", nullptr) };
  toggle_action->connect_activate(this, &Application::on_toggle);
  add_action(toggle_action);

  const auto quit_action{ Gio::SimpleAction::create("quit-action", nullptr) };
  quit_action->connect_activate(this, &Application::on_quit);
  add_action(quit_action);

  m_css_provider = Gtk::CssProvider::create();
  m_css_provider->load_from_file(m_config->css_file());
  Gtk::StyleContext::add_provider_for_display(
    Gdk::Display::get_default(), m_css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
}

void Application::start_worker_thread()
{
  GLib::Thread::create("vin_worker", [this]() -> gpointer {
    spdlog::info("worker thread running");
    m_worker_context->push_thread_default();
    UniquePtr<GLib::Error> error;
    m_config->monitor_css(nullptr, &error);
    if (error) {
      spdlog::error("{}", error->message);
    }
    m_config->monitor_config(nullptr, &error);
    if (error) {
      spdlog::error("{}", error->message);
    }
    const auto worker_loop{ GLib::MainLoop::create(m_worker_context, false) };
    worker_loop->run();
    return nullptr;
  });
}

void Application::on_activate([[maybe_unused]] Gio::Application* const app)
{
  if (m_exit_status > 0 || get_active_window() != nullptr) {
    return;
  }

  spdlog::info("vin activate");

  UniquePtr<GLib::Error> error;

  auto* const window{ Window::create(this, m_main_context, m_worker_context) };

  m_config->init_state();

  window->cast<lib::IConfigurable>()->configure(m_config->state());

  m_config->finish_init_state();

  if (!m_config->load_config(&error)) {
    spdlog::error("{}", error->message);
    return;
  }

  spdlog::info("showing window");

  window->present();
}

void Application::on_shutdown( // NOLINT(readability-convert-member-functions-to-static)
  [[maybe_unused]] Gio::Application* const app)
{
  spdlog::info("vin shutdown");
}

int Application::on_handle_local_options( // NOLINT(readability-make-member-function-const)
  [[maybe_unused]] Gio::Application* const app,
  [[maybe_unused]] GLib::VariantDict* const dict)
{
  if (m_arg_version) {
    fmt::println("0.0.0 (TODO)");
    return 0;
  }

  return -1;
}

bool Application::vfunc_local_command_line(Strv* const args, int* const exit_status)
{
  const bool handled_all{ parent_vfunc_local_command_line<Application>(args, exit_status) };

  if (m_exit_status > 0) {
    *exit_status = m_exit_status;
    return true;
  }

  if (m_arg_quit) {
    cast<Gio::ActionGroup>()->activate_action("quit-action", nullptr);
  } else if (m_arg_toggle) {
    cast<Gio::ActionGroup>()->activate_action("toggle-action", nullptr);
  }

  return handled_all;
}

void Application::on_toggle([[maybe_unused]] Gio::SimpleAction* const action,
  [[maybe_unused]] GLib::Variant* const variant)
{
  auto* const window{ get_active_window() };
  g_assert(window != nullptr);
  window->set_visible(!window->get_visible());
}

void Application::on_quit([[maybe_unused]] Gio::SimpleAction* const action,
  [[maybe_unused]] GLib::Variant* const variant)
{
  RefPtr<Gtk::Window>::adopt_ref(get_active_window());
}

void Application::on_css_changed(Config* const config)
{
  m_css_provider->load_from_file(config->css_file());
}

void Application::on_config_changed([[maybe_unused]] Config* const config)
{
  auto* const window{ get_active_window() };
  if (window == nullptr) {
    return;
  }
  UniquePtr<GLib::Error> error;
  if (!m_config->load_config(&error)) {
    spdlog::error("{}", error->message);
  }
}
