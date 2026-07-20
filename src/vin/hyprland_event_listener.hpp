#pragma once

#include "vin/main_context.hpp"

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

namespace vin {

class HyprlandEventListener final : public peel::Gio::Initable
{
  PEEL_SIMPLE_CLASS(HyprlandEventListener, Initable)

  friend class peel::Gio::Initable;

  using Event = peel::Signal<HyprlandEventListener, void(const char*)>;

  static Event s_pin;
  static Event s_bell;
  static Event s_kill;
  static Event s_urgent;
  static Event s_submap;
  static Event s_minimized;
  static Event s_openlayer;
  static Event s_lockgroups;
  static Event s_fullscreen;
  static Event s_closelayer;
  static Event s_openwindow;
  static Event s_workspacev2;
  static Event s_togglegroup;
  static Event s_closewindow;
  static Event s_activelayout;
  static Event s_focusedmonv2;
  static Event s_movewindowv2;
  static Event s_screencastv2;
  static Event s_windowtitlev2;
  static Event s_moveintogroup;
  static Event s_moveoutofgroup;
  static Event s_configreloaded;
  static Event s_activewindowv2;
  static Event s_monitoraddedv2;
  static Event s_renameworkspace;
  static Event s_activespecialv2;
  static Event s_moveworkspacev2;
  static Event s_ignoregrouplock;
  static Event s_monitorremovedv2;
  static Event s_createworkspacev2;
  static Event s_destroyworkspacev2;
  static Event s_changefloatingmode;

  peel::GLib::MainContext* m_main_context;
  peel::GLib::MainContext* m_worker_context;
  peel::RefPtr<peel::GLib::Source> m_event_source;
  int m_socket_fd;

public:
  ~HyprlandEventListener();

  static auto create(const MainContext context)
  {
    return peel::Object::create<HyprlandEventListener>(
      prop_main_context(), context.main_context, prop_worker_context(), context.worker_context);
  }

  VIN_MAIN_CONTEXT_PROPERTY

  PEEL_SIGNAL_CONNECT_METHOD(pin, s_pin)
  PEEL_SIGNAL_CONNECT_METHOD(bell, s_bell)
  PEEL_SIGNAL_CONNECT_METHOD(kill, s_kill)
  PEEL_SIGNAL_CONNECT_METHOD(urgent, s_urgent)
  PEEL_SIGNAL_CONNECT_METHOD(submap, s_submap)
  PEEL_SIGNAL_CONNECT_METHOD(minimized, s_minimized)
  PEEL_SIGNAL_CONNECT_METHOD(openlayer, s_openlayer)
  PEEL_SIGNAL_CONNECT_METHOD(lockgroups, s_lockgroups)
  PEEL_SIGNAL_CONNECT_METHOD(fullscreen, s_fullscreen)
  PEEL_SIGNAL_CONNECT_METHOD(closelayer, s_closelayer)
  PEEL_SIGNAL_CONNECT_METHOD(openwindow, s_openwindow)
  PEEL_SIGNAL_CONNECT_METHOD(workspacev2, s_workspacev2)
  PEEL_SIGNAL_CONNECT_METHOD(togglegroup, s_togglegroup)
  PEEL_SIGNAL_CONNECT_METHOD(closewindow, s_closewindow)
  PEEL_SIGNAL_CONNECT_METHOD(activelayout, s_activelayout)
  PEEL_SIGNAL_CONNECT_METHOD(focusedmonv2, s_focusedmonv2)
  PEEL_SIGNAL_CONNECT_METHOD(movewindowv2, s_movewindowv2)
  PEEL_SIGNAL_CONNECT_METHOD(screencastv2, s_screencastv2)
  PEEL_SIGNAL_CONNECT_METHOD(windowtitlev2, s_windowtitlev2)
  PEEL_SIGNAL_CONNECT_METHOD(moveintogroup, s_moveintogroup)
  PEEL_SIGNAL_CONNECT_METHOD(moveoutofgroup, s_moveoutofgroup)
  PEEL_SIGNAL_CONNECT_METHOD(configreloaded, s_configreloaded)
  PEEL_SIGNAL_CONNECT_METHOD(activewindowv2, s_activewindowv2)
  PEEL_SIGNAL_CONNECT_METHOD(monitoraddedv2, s_monitoraddedv2)
  PEEL_SIGNAL_CONNECT_METHOD(renameworkspace, s_renameworkspace)
  PEEL_SIGNAL_CONNECT_METHOD(activespecialv2, s_activespecialv2)
  PEEL_SIGNAL_CONNECT_METHOD(moveworkspacev2, s_moveworkspacev2)
  PEEL_SIGNAL_CONNECT_METHOD(ignoregrouplock, s_ignoregrouplock)
  PEEL_SIGNAL_CONNECT_METHOD(monitorremovedv2, s_monitorremovedv2)
  PEEL_SIGNAL_CONNECT_METHOD(createworkspacev2, s_createworkspacev2)
  PEEL_SIGNAL_CONNECT_METHOD(destroyworkspacev2, s_destroyworkspacev2)
  PEEL_SIGNAL_CONNECT_METHOD(changefloatingmode, s_changefloatingmode)

private:
  static void init_type(peel::Type type);

  static void init_interface(peel::Gio::Initable::Iface* iface);

  bool vfunc_init(peel::Gio::Cancellable* cancellable, peel::UniquePtr<peel::GLib::Error>* error);

  void vfunc_dispose();

  static void define_properties(auto& visitor);
};

} // namespace vin
