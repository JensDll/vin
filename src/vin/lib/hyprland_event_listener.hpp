#pragma once

#include "vin/peel/gio.hpp"

#include <peel/GObject/Object.h>
#include <peel/GObject/ParamFlags.h>
#include <peel/class.h>
#include <peel/Gio/Cancellable.h>
#include <peel/Gio/Initable.h>
#include <peel/GLib/Error.h>
#include <peel/GLib/MainContext.h>
#include <peel/property.h>
#include <peel/RefPtr.h>
#include <peel/signal.h>
#include <peel/UniquePtr.h>

namespace vin::lib {

class HyprlandEventListener final : public peel::Gio::Initable
{
private:
  PEEL_SIMPLE_CLASS(HyprlandEventListener, Initable)

  friend class peel::Gio::Initable;

  using SignalEvent = peel::Signal<HyprlandEventListener, void(const char*)>;

  static SignalEvent s_signal_pin;
  static SignalEvent s_signal_bell;
  static SignalEvent s_signal_kill;
  static SignalEvent s_signal_urgent;
  static SignalEvent s_signal_submap;
  static SignalEvent s_signal_minimized;
  static SignalEvent s_signal_openlayer;
  static SignalEvent s_signal_lockgroups;
  static SignalEvent s_signal_fullscreen;
  static SignalEvent s_signal_closelayer;
  static SignalEvent s_signal_openwindow;
  static SignalEvent s_signal_workspacev2;
  static SignalEvent s_signal_togglegroup;
  static SignalEvent s_signal_closewindow;
  static SignalEvent s_signal_activelayout;
  static SignalEvent s_signal_focusedmonv2;
  static SignalEvent s_signal_movewindowv2;
  static SignalEvent s_signal_screencastv2;
  static SignalEvent s_signal_windowtitlev2;
  static SignalEvent s_signal_moveintogroup;
  static SignalEvent s_signal_moveoutofgroup;
  static SignalEvent s_signal_configreloaded;
  static SignalEvent s_signal_activewindowv2;
  static SignalEvent s_signal_monitoraddedv2;
  static SignalEvent s_signal_renameworkspace;
  static SignalEvent s_signal_activespecialv2;
  static SignalEvent s_signal_moveworkspacev2;
  static SignalEvent s_signal_ignoregrouplock;
  static SignalEvent s_signal_monitorremovedv2;
  static SignalEvent s_signal_createworkspacev2;
  static SignalEvent s_signal_destroyworkspacev2;
  static SignalEvent s_signal_changefloatingmode;

  int m_socket_fd;
  peel::GLib::MainContext* m_main_context;
  peel::GLib::MainContext* m_worker_context;
  peel::RefPtr<peel::GLib::Source> m_event_source;

public:
  ~HyprlandEventListener();

  static auto create(peel::Gio::Cancellable* const cancellable,
    peel::UniquePtr<peel::GLib::Error>* const error,
    peel::GLib::MainContext* const main_context,
    peel::GLib::MainContext* const worker_context)
  {
    return vin::gio::initable_create<HyprlandEventListener>(
      cancellable, error, prop_main_context(), main_context, prop_worker_context(), worker_context);
  }

  static auto singleton()
  {
    return peel::Object::create<HyprlandEventListener>();
  }

  PEEL_SIGNAL_CONNECT_METHOD(pin, s_signal_pin)
  PEEL_SIGNAL_CONNECT_METHOD(bell, s_signal_bell)
  PEEL_SIGNAL_CONNECT_METHOD(kill, s_signal_kill)
  PEEL_SIGNAL_CONNECT_METHOD(urgent, s_signal_urgent)
  PEEL_SIGNAL_CONNECT_METHOD(submap, s_signal_submap)
  PEEL_SIGNAL_CONNECT_METHOD(minimized, s_signal_minimized)
  PEEL_SIGNAL_CONNECT_METHOD(openlayer, s_signal_openlayer)
  PEEL_SIGNAL_CONNECT_METHOD(lockgroups, s_signal_lockgroups)
  PEEL_SIGNAL_CONNECT_METHOD(fullscreen, s_signal_fullscreen)
  PEEL_SIGNAL_CONNECT_METHOD(closelayer, s_signal_closelayer)
  PEEL_SIGNAL_CONNECT_METHOD(openwindow, s_signal_openwindow)
  PEEL_SIGNAL_CONNECT_METHOD(workspacev2, s_signal_workspacev2)
  PEEL_SIGNAL_CONNECT_METHOD(togglegroup, s_signal_togglegroup)
  PEEL_SIGNAL_CONNECT_METHOD(closewindow, s_signal_closewindow)
  PEEL_SIGNAL_CONNECT_METHOD(activelayout, s_signal_activelayout)
  PEEL_SIGNAL_CONNECT_METHOD(focusedmonv2, s_signal_focusedmonv2)
  PEEL_SIGNAL_CONNECT_METHOD(movewindowv2, s_signal_movewindowv2)
  PEEL_SIGNAL_CONNECT_METHOD(screencastv2, s_signal_screencastv2)
  PEEL_SIGNAL_CONNECT_METHOD(windowtitlev2, s_signal_windowtitlev2)
  PEEL_SIGNAL_CONNECT_METHOD(moveintogroup, s_signal_moveintogroup)
  PEEL_SIGNAL_CONNECT_METHOD(moveoutofgroup, s_signal_moveoutofgroup)
  PEEL_SIGNAL_CONNECT_METHOD(configreloaded, s_signal_configreloaded)
  PEEL_SIGNAL_CONNECT_METHOD(activewindowv2, s_signal_activewindowv2)
  PEEL_SIGNAL_CONNECT_METHOD(monitoraddedv2, s_signal_monitoraddedv2)
  PEEL_SIGNAL_CONNECT_METHOD(renameworkspace, s_signal_renameworkspace)
  PEEL_SIGNAL_CONNECT_METHOD(activespecialv2, s_signal_activespecialv2)
  PEEL_SIGNAL_CONNECT_METHOD(moveworkspacev2, s_signal_moveworkspacev2)
  PEEL_SIGNAL_CONNECT_METHOD(ignoregrouplock, s_signal_ignoregrouplock)
  PEEL_SIGNAL_CONNECT_METHOD(monitorremovedv2, s_signal_monitorremovedv2)
  PEEL_SIGNAL_CONNECT_METHOD(createworkspacev2, s_signal_createworkspacev2)
  PEEL_SIGNAL_CONNECT_METHOD(destroyworkspacev2, s_signal_destroyworkspacev2)
  PEEL_SIGNAL_CONNECT_METHOD(changefloatingmode, s_signal_changefloatingmode)

private:
  static void init_type(peel::Type type);

  static void init_interface(peel::Gio::Initable::Iface* iface);

  bool vfunc_init(peel::Gio::Cancellable* cancellable, peel::UniquePtr<peel::GLib::Error>* error);

  static peel::RefPtr<peel::Object> vfunc_constructor(peel::Type type,
    peel::ArrayRef<peel::Object::ConstructParam> params);

  void vfunc_dispose();

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
    visitor.prop(prop_main_context())
      .set(&HyprlandEventListener::set_main_context)
      .flags(peel::GObject::ParamFlags::CONSTRUCT_ONLY);
    visitor.prop(prop_worker_context())
      .set(&HyprlandEventListener::set_worker_context)
      .flags(peel::GObject::ParamFlags::CONSTRUCT_ONLY);
  }
};

} // namespace vin::lib
