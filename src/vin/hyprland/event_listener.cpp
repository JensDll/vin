#include "vin/hyprland/connect.hpp"
#include "vin/hyprland/event_listener.hpp"
#include "vin/peel/patch.hpp"

#include <peel/GObject/Object.h>
#include <peel/GObject/Type.h>
#include <fmt/format.h>
#include <glib.h>
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
using namespace vin::hyprland;

PEEL_CLASS_IMPL(EventListener, "VinHyprlandEventListener", peel::GObject::Object)

EventListener::SignalEvent EventListener::s_signal_pin = EventListener::SignalEvent::create("pin");
EventListener::SignalEvent EventListener::s_signal_bell = EventListener::SignalEvent::create("bell");
EventListener::SignalEvent EventListener::s_signal_kill = EventListener::SignalEvent::create("kill");
EventListener::SignalEvent EventListener::s_signal_urgent = EventListener::SignalEvent::create("urgent");
EventListener::SignalEvent EventListener::s_signal_submap = EventListener::SignalEvent::create("submap");
EventListener::SignalEvent EventListener::s_signal_minimized = EventListener::SignalEvent::create("minimized");
EventListener::SignalEvent EventListener::s_signal_openlayer = EventListener::SignalEvent::create("openlayer");
EventListener::SignalEvent EventListener::s_signal_lockgroups = EventListener::SignalEvent::create("lockgroups");
EventListener::SignalEvent EventListener::s_signal_fullscreen = EventListener::SignalEvent::create("fullscreen");
EventListener::SignalEvent EventListener::s_signal_closelayer = EventListener::SignalEvent::create("closelayer");
EventListener::SignalEvent EventListener::s_signal_openwindow = EventListener::SignalEvent::create("openwindow");
EventListener::SignalEvent EventListener::s_signal_workspacev2 = EventListener::SignalEvent::create("workspacev2");
EventListener::SignalEvent EventListener::s_signal_togglegroup = EventListener::SignalEvent::create("togglegroup");
EventListener::SignalEvent EventListener::s_signal_closewindow = EventListener::SignalEvent::create("closewindow");
EventListener::SignalEvent EventListener::s_signal_activelayout = EventListener::SignalEvent::create("activelayout");
EventListener::SignalEvent EventListener::s_signal_focusedmonv2 = EventListener::SignalEvent::create("focusedmonv2");
EventListener::SignalEvent EventListener::s_signal_movewindowv2 = EventListener::SignalEvent::create("movewindowv2");
EventListener::SignalEvent EventListener::s_signal_screencastv2 = EventListener::SignalEvent::create("screencastv2");
EventListener::SignalEvent EventListener::s_signal_windowtitlev2 = EventListener::SignalEvent::create("windowtitlev2");
EventListener::SignalEvent EventListener::s_signal_moveintogroup = EventListener::SignalEvent::create("moveintogroup");
EventListener::SignalEvent EventListener::s_signal_moveoutofgroup =
  EventListener::SignalEvent::create("moveoutofgroup");
EventListener::SignalEvent EventListener::s_signal_configreloaded =
  EventListener::SignalEvent::create("configreloaded");
EventListener::SignalEvent EventListener::s_signal_activewindowv2 =
  EventListener::SignalEvent::create("activewindowv2");
EventListener::SignalEvent EventListener::s_signal_monitoraddedv2 =
  EventListener::SignalEvent::create("monitoraddedv2");
EventListener::SignalEvent EventListener::s_signal_renameworkspace =
  EventListener::SignalEvent::create("renameworkspace");
EventListener::SignalEvent EventListener::s_signal_activespecialv2 =
  EventListener::SignalEvent::create("activespecialv2");
EventListener::SignalEvent EventListener::s_signal_moveworkspacev2 =
  EventListener::SignalEvent::create("moveworkspacev2");
EventListener::SignalEvent EventListener::s_signal_ignoregrouplock =
  EventListener::SignalEvent::create("ignoregrouplock");
EventListener::SignalEvent EventListener::s_signal_monitorremovedv2 =
  EventListener::SignalEvent::create("monitorremovedv2");
EventListener::SignalEvent EventListener::s_signal_createworkspacev2 =
  EventListener::SignalEvent::create("createworkspacev2");
EventListener::SignalEvent EventListener::s_signal_destroyworkspacev2 =
  EventListener::SignalEvent::create("destroyworkspacev2");
EventListener::SignalEvent EventListener::s_signal_changefloatingmode =
  EventListener::SignalEvent::create("changefloatingmode");

EventListener::~EventListener()
{
  spdlog::debug("hyprland event listener finalize (fd = {})", m_socket_fd);
  if (m_socket_fd != -1) {
    close(m_socket_fd);
  }
}

void EventListener::init_type(const Type type)
{
  PEEL_IMPLEMENT_INTERFACE(type, Gio::Initable);
}

void EventListener::init_interface(Gio::Initable::Iface* const iface)
{
  iface->override_vfunc_init<EventListener>();
}

void EventListener::Class::init()
{
  override_vfunc_constructor<EventListener>();
}

RefPtr<Object> EventListener::vfunc_constructor(const Type type, const ArrayRef<Object::ConstructParam> params)
{
  static Object* self{};

  if (self == nullptr) {
    const auto new_self{ parent_vfunc_constructor<EventListener>(type, params) };
    new_self->add_weak_pointer(&self);
    spdlog::debug("new hyprland event listener {}", static_cast<const void*>(new_self));
    return self = new_self;
  }

  spdlog::debug("reuse hyprland event listener {}", static_cast<const void*>(self));

  return self;
}

#define TRY_EMIT_EVENT(name)                                           \
  if (event.starts_with(#name ">>")) {                                 \
    m_main_context->invoke([this, event]() {                           \
      s_signal_##name.emit(this, event.data() + sizeof(#name) + 1);    \
      /* NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) */      \
      GLib::free_sized(const_cast<char*>(event.data()), event.size()); \
      return G_SOURCE_REMOVE;                                          \
    });                                                                \
    return G_SOURCE_CONTINUE;                                          \
  }

bool EventListener::vfunc_init([[maybe_unused]] Gio::Cancellable* const cancellable,
  UniquePtr<GLib::Error>* const error)
{
  spdlog::debug("hyprland event listener init {}", static_cast<const void*>(this));

  m_socket_fd = connect<Connect::receive>(error);

  if (m_socket_fd == -1) {
    return false;
  }

  const auto channel{ GLib::IOChannel::unix_new(m_socket_fd) };
  const auto event_source{ GLib::io_create_watch(channel, GLib::IOCondition::IN_) };

  vin::source_set_io_callback(
    event_source, [this](GLib::IOChannel* const the_channel, [[maybe_unused]] const GLib::IOCondition condition) {
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
      GLib::free_sized(const_cast<char*>(event.data()), event.size());
      return G_SOURCE_CONTINUE;
    });

  event_source->attach(m_worker_context);

  return true;
}
