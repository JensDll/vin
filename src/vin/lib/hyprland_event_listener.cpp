#include "vin/lib/hyprland_connect.hpp"
#include "vin/lib/hyprland_event_listener.hpp"
#include "vin/peel/glib.hpp"

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
using namespace vin::lib;

PEEL_CLASS_IMPL(HyprlandEventListener, "VinHyprlandEventListener", peel::GObject::Object)

HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_pin;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_bell;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_kill;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_urgent;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_submap;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_minimized;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_openlayer;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_lockgroups;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_fullscreen;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_closelayer;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_openwindow;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_workspacev2;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_togglegroup;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_closewindow;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_activelayout;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_focusedmonv2;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_movewindowv2;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_screencastv2;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_windowtitlev2;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_moveintogroup;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_moveoutofgroup;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_configreloaded;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_activewindowv2;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_monitoraddedv2;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_renameworkspace;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_activespecialv2;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_moveworkspacev2;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_ignoregrouplock;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_monitorremovedv2;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_createworkspacev2;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_destroyworkspacev2;
HyprlandEventListener::SignalEvent HyprlandEventListener::s_signal_changefloatingmode;

HyprlandEventListener::~HyprlandEventListener()
{
  spdlog::info("finalize hyprland event listener {}", m_socket_fd);
  if (m_socket_fd != -1) {
    close(m_socket_fd);
  }
}

void HyprlandEventListener::Class::init()
{
  s_signal_pin = SignalEvent::create("pin");
  s_signal_bell = SignalEvent::create("bell");
  s_signal_kill = SignalEvent::create("kill");
  s_signal_urgent = SignalEvent::create("urgent");
  s_signal_submap = SignalEvent::create("submap");
  s_signal_minimized = SignalEvent::create("minimized");
  s_signal_openlayer = SignalEvent::create("openlayer");
  s_signal_lockgroups = SignalEvent::create("lockgroups");
  s_signal_fullscreen = SignalEvent::create("fullscreen");
  s_signal_closelayer = SignalEvent::create("closelayer");
  s_signal_openwindow = SignalEvent::create("openwindow");
  s_signal_workspacev2 = SignalEvent::create("workspacev2");
  s_signal_togglegroup = SignalEvent::create("togglegroup");
  s_signal_closewindow = SignalEvent::create("closewindow");
  s_signal_activelayout = SignalEvent::create("activelayout");
  s_signal_focusedmonv2 = SignalEvent::create("focusedmonv2");
  s_signal_movewindowv2 = SignalEvent::create("movewindowv2");
  s_signal_screencastv2 = SignalEvent::create("screencastv2");
  s_signal_windowtitlev2 = SignalEvent::create("windowtitlev2");
  s_signal_moveintogroup = SignalEvent::create("moveintogroup");
  s_signal_moveoutofgroup = SignalEvent::create("moveoutofgroup");
  s_signal_configreloaded = SignalEvent::create("configreloaded");
  s_signal_activewindowv2 = SignalEvent::create("activewindowv2");
  s_signal_monitoraddedv2 = SignalEvent::create("monitoraddedv2");
  s_signal_renameworkspace = SignalEvent::create("renameworkspace");
  s_signal_activespecialv2 = SignalEvent::create("activespecialv2");
  s_signal_moveworkspacev2 = SignalEvent::create("moveworkspacev2");
  s_signal_ignoregrouplock = SignalEvent::create("ignoregrouplock");
  s_signal_monitorremovedv2 = SignalEvent::create("monitorremovedv2");
  s_signal_createworkspacev2 = SignalEvent::create("createworkspacev2");
  s_signal_destroyworkspacev2 = SignalEvent::create("destroyworkspacev2");
  s_signal_changefloatingmode = SignalEvent::create("changefloatingmode");
  override_vfunc_constructor<HyprlandEventListener>();
  override_vfunc_dispose<HyprlandEventListener>();
}

void HyprlandEventListener::vfunc_dispose()
{
  spdlog::info("dispose hyprland event listener");
  if (m_event_source != nullptr) {
    m_event_source->destroy();
    m_event_source = nullptr;
  }
  parent_vfunc_dispose<HyprlandEventListener>();
}

RefPtr<Object> HyprlandEventListener::vfunc_constructor(const Type type, const ArrayRef<Object::ConstructParam> params)
{
  static Object* self{};

  if (self == nullptr) {
    const auto new_self{ parent_vfunc_constructor<HyprlandEventListener>(type, params) };
    new_self->add_weak_pointer(&self);
    spdlog::info("new hyprland event listener");
    return self = new_self;
  }

  spdlog::info("reuse hyprland event listener");

  return self;
}

#define TRY_EMIT_EVENT(name)                                               \
  if (event.starts_with(#name ">>")) {                                     \
    m_main_context->invoke([this, event]() {                               \
      s_signal_##name.emit(this, event.data() + sizeof(#name) + 1);        \
      /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) */          \
      GLib::free_sized(const_cast<char*>(event.data()), event.size() + 1); \
      return G_SOURCE_REMOVE;                                              \
    });                                                                    \
    return G_SOURCE_CONTINUE;                                              \
  }

bool HyprlandEventListener::vfunc_init([[maybe_unused]] Gio::Cancellable* const cancellable,
  UniquePtr<GLib::Error>* const error)
{
  spdlog::info("init hyprland event listener");

  m_socket_fd = connect_hyprland<Connect::receive>(error);

  if (m_socket_fd == -1) {
    return false;
  }

  const auto channel{ GLib::IOChannel::unix_new(m_socket_fd) };

  m_event_source = GLib::io_create_watch(channel, GLib::IOCondition::IN_);

  vin::glib::source_set_io_callback(
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
