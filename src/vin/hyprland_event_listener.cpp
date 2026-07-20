#include "vin/gsource.hpp"
#include "vin/hyprland_connect.hpp"
#include "vin/hyprland_event_listener.hpp"
#include "vin/main_context.hpp"

#include <peel/GObject/Object.h>
#include <peel/GObject/Type.h>
#include <fmt/format.h>
#include <peel/Gio/Initable.h>
#include <peel/GLib/Error.h>
#include <peel/GLib/functions.h>
#include <peel/GLib/IOChannel.h>
#include <peel/GLib/IOCondition.h>
#include <peel/GLib/MainContext.h>
#include <peel/GLib/Quark.h>
#include <peel/RefPtr.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <string_view>

using namespace peel;
using namespace vin;

PEEL_CLASS_IMPL(HyprlandEventListener, "VinHyprlandEventListener", peel::GObject::Object)

HyprlandEventListener::Event HyprlandEventListener::s_pin;
HyprlandEventListener::Event HyprlandEventListener::s_bell;
HyprlandEventListener::Event HyprlandEventListener::s_kill;
HyprlandEventListener::Event HyprlandEventListener::s_urgent;
HyprlandEventListener::Event HyprlandEventListener::s_submap;
HyprlandEventListener::Event HyprlandEventListener::s_minimized;
HyprlandEventListener::Event HyprlandEventListener::s_openlayer;
HyprlandEventListener::Event HyprlandEventListener::s_lockgroups;
HyprlandEventListener::Event HyprlandEventListener::s_fullscreen;
HyprlandEventListener::Event HyprlandEventListener::s_closelayer;
HyprlandEventListener::Event HyprlandEventListener::s_openwindow;
HyprlandEventListener::Event HyprlandEventListener::s_workspacev2;
HyprlandEventListener::Event HyprlandEventListener::s_togglegroup;
HyprlandEventListener::Event HyprlandEventListener::s_closewindow;
HyprlandEventListener::Event HyprlandEventListener::s_activelayout;
HyprlandEventListener::Event HyprlandEventListener::s_focusedmonv2;
HyprlandEventListener::Event HyprlandEventListener::s_movewindowv2;
HyprlandEventListener::Event HyprlandEventListener::s_screencastv2;
HyprlandEventListener::Event HyprlandEventListener::s_windowtitlev2;
HyprlandEventListener::Event HyprlandEventListener::s_moveintogroup;
HyprlandEventListener::Event HyprlandEventListener::s_moveoutofgroup;
HyprlandEventListener::Event HyprlandEventListener::s_configreloaded;
HyprlandEventListener::Event HyprlandEventListener::s_activewindowv2;
HyprlandEventListener::Event HyprlandEventListener::s_monitoraddedv2;
HyprlandEventListener::Event HyprlandEventListener::s_renameworkspace;
HyprlandEventListener::Event HyprlandEventListener::s_activespecialv2;
HyprlandEventListener::Event HyprlandEventListener::s_moveworkspacev2;
HyprlandEventListener::Event HyprlandEventListener::s_ignoregrouplock;
HyprlandEventListener::Event HyprlandEventListener::s_monitorremovedv2;
HyprlandEventListener::Event HyprlandEventListener::s_createworkspacev2;
HyprlandEventListener::Event HyprlandEventListener::s_destroyworkspacev2;
HyprlandEventListener::Event HyprlandEventListener::s_changefloatingmode;

HyprlandEventListener::~HyprlandEventListener()
{
  spdlog::info("[VinHyprlandEventListener] finalize");
  if (m_socket_fd != -1) {
    close(m_socket_fd);
  }
}

void HyprlandEventListener::Class::init()
{
  s_pin = Event::create("pin");
  s_bell = Event::create("bell");
  s_kill = Event::create("kill");
  s_urgent = Event::create("urgent");
  s_submap = Event::create("submap");
  s_minimized = Event::create("minimized");
  s_openlayer = Event::create("openlayer");
  s_lockgroups = Event::create("lockgroups");
  s_fullscreen = Event::create("fullscreen");
  s_closelayer = Event::create("closelayer");
  s_openwindow = Event::create("openwindow");
  s_workspacev2 = Event::create("workspacev2");
  s_togglegroup = Event::create("togglegroup");
  s_closewindow = Event::create("closewindow");
  s_activelayout = Event::create("activelayout");
  s_focusedmonv2 = Event::create("focusedmonv2");
  s_movewindowv2 = Event::create("movewindowv2");
  s_screencastv2 = Event::create("screencastv2");
  s_windowtitlev2 = Event::create("windowtitlev2");
  s_moveintogroup = Event::create("moveintogroup");
  s_moveoutofgroup = Event::create("moveoutofgroup");
  s_configreloaded = Event::create("configreloaded");
  s_activewindowv2 = Event::create("activewindowv2");
  s_monitoraddedv2 = Event::create("monitoraddedv2");
  s_renameworkspace = Event::create("renameworkspace");
  s_activespecialv2 = Event::create("activespecialv2");
  s_moveworkspacev2 = Event::create("moveworkspacev2");
  s_ignoregrouplock = Event::create("ignoregrouplock");
  s_monitorremovedv2 = Event::create("monitorremovedv2");
  s_createworkspacev2 = Event::create("createworkspacev2");
  s_destroyworkspacev2 = Event::create("destroyworkspacev2");
  s_changefloatingmode = Event::create("changefloatingmode");
  override_vfunc_dispose<HyprlandEventListener>();
}

void HyprlandEventListener::vfunc_dispose()
{
  spdlog::info("[VinHyprlandEventListener] dispose");
  if (m_event_source != nullptr) {
    m_event_source->destroy();
    m_event_source = nullptr;
  }
  parent_vfunc_dispose<HyprlandEventListener>();
}

#define TRY_EMIT_EVENT(name)                                               \
  if (event.starts_with(#name ">>")) {                                     \
    m_main_context->invoke([this, event]() {                               \
      s_##name.emit(this, event.data() + sizeof(#name) + 1);               \
      /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) */          \
      GLib::free_sized(const_cast<char*>(event.data()), event.size() + 1); \
      return G_SOURCE_REMOVE;                                              \
    });                                                                    \
    return G_SOURCE_CONTINUE;                                              \
  }

bool HyprlandEventListener::vfunc_init([[maybe_unused]] Gio::Cancellable* const cancellable,
  UniquePtr<GLib::Error>* const error)
{
  m_socket_fd = connect_hyprland<Connect::receive>(error);

  if (m_socket_fd == -1) {
    return false;
  }

  const auto channel{ GLib::IOChannel::unix_new(m_socket_fd) };

  m_event_source = GLib::io_create_watch(channel, GLib::IOCondition::IN_);

  vin::g_source_set_io_callback(
    m_event_source, [this](GLib::IOChannel* const the_channel, [[maybe_unused]] const GLib::IOCondition condition) {
      String line;
      std::size_t length; // NOLINT(cppcoreguidelines-init-variables)
      the_channel->read_line(&line, &length, nullptr, nullptr);
      const std::string_view event(std::move(line).release_string(), length);
      TRY_EMIT_EVENT(pin)
      TRY_EMIT_EVENT(bell)
      TRY_EMIT_EVENT(kill)
      TRY_EMIT_EVENT(urgent)
      TRY_EMIT_EVENT(submap)
      TRY_EMIT_EVENT(minimized)
      TRY_EMIT_EVENT(openlayer)
      TRY_EMIT_EVENT(lockgroups)
      TRY_EMIT_EVENT(fullscreen)
      TRY_EMIT_EVENT(closelayer)
      TRY_EMIT_EVENT(openwindow)
      TRY_EMIT_EVENT(togglegroup)
      TRY_EMIT_EVENT(closewindow)
      TRY_EMIT_EVENT(workspacev2)
      TRY_EMIT_EVENT(activelayout)
      TRY_EMIT_EVENT(focusedmonv2)
      TRY_EMIT_EVENT(movewindowv2)
      TRY_EMIT_EVENT(screencastv2)
      TRY_EMIT_EVENT(windowtitlev2)
      TRY_EMIT_EVENT(moveintogroup)
      TRY_EMIT_EVENT(moveoutofgroup)
      TRY_EMIT_EVENT(configreloaded)
      TRY_EMIT_EVENT(activewindowv2)
      TRY_EMIT_EVENT(monitoraddedv2)
      TRY_EMIT_EVENT(renameworkspace)
      TRY_EMIT_EVENT(activespecialv2)
      TRY_EMIT_EVENT(moveworkspacev2)
      TRY_EMIT_EVENT(ignoregrouplock)
      TRY_EMIT_EVENT(monitorremovedv2)
      TRY_EMIT_EVENT(createworkspacev2)
      TRY_EMIT_EVENT(destroyworkspacev2)
      TRY_EMIT_EVENT(changefloatingmode)
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
      GLib::free_sized(const_cast<char*>(event.data()), event.size() + 1);
      return G_SOURCE_CONTINUE;
    });

  m_event_source->attach(m_worker_context);

  return true;
}

void HyprlandEventListener::init_type(const Type type)
{
  PEEL_IMPLEMENT_INTERFACE(type, Gio::Initable);
}

void HyprlandEventListener::init_interface(Gio::Initable::Iface* const iface)
{
  iface->override_vfunc_init<HyprlandEventListener>();
}

VIN_IMPLEMENT_MAIN_CONTEXT_PROPERTY(HyprlandEventListener)

void HyprlandEventListener::define_properties(auto& visitor)
{
  VIN_DEFINE_MAIN_CONTEXT_PROPERTY(HyprlandEventListener)
}
