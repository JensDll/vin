#pragma once

#include "vin/lib/hyprland_workspace_list_model.hpp"
#include "vin/lib/iconfigurable.hpp"
#include "vin/lib/ipositionable.hpp"
#include "vin/peel/gio.hpp"

#include <peel/class.h>
#include <peel/Gio/Initable.h>
#include <peel/Gtk/Box.h>
#include <peel/Gtk/ListView.h>
#include <peel/Gtk/MenuButton.h>
#include <peel/Gtk/Orientable.h>
#include <peel/Gtk/Orientation.h>
#include <peel/RefPtr.h>

namespace vin::module {

class Workspace final : public peel::Gtk::Box
{
private:
  PEEL_SIMPLE_CLASS(Workspace, Box)

  friend class lib::IPositionable;
  friend class lib::IConfigurable;
  friend class peel::Gio::Initable;

  peel::GLib::MainContext* m_main_context;
  peel::GLib::MainContext* m_worker_context;

  peel::Gtk::ListView* m_normal_view;

public:
  ~Workspace() = default;

  static auto create(peel::Gio::Cancellable* const cancellable,
    peel::UniquePtr<peel::GLib::Error>* const error,
    peel::GLib::MainContext* const main_context,
    peel::GLib::MainContext* const worker_context)
  {
    return vin::gio::initable_create<Workspace>(cancellable,
      error,
      peel::Gtk::Orientable::prop_orientation(),
      peel::Gtk::Orientation::HORIZONTAL,
      prop_spacing(),
      0,
      prop_main_context(),
      main_context,
      prop_worker_context(),
      worker_context);
  }

private:
  static void init_type(peel::Type type);

  static void init_interface(lib::IPositionable::Iface* iface);

  static void init_interface(lib::IConfigurable::Iface* iface);

  static void init_interface(peel::Gio::Initable::Iface* iface);

  bool vfunc_init(peel::Gio::Cancellable* cancellable, peel::UniquePtr<peel::GLib::Error>* error);

  void vfunc_dispose();

  void on_no_special(lib::HyprlandWorkspaceListModel* model);

  void vfunc_position(lib::Position position);

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
      .set(&Workspace::set_main_context)
      .flags(peel::GObject::ParamFlags::CONSTRUCT_ONLY);
    visitor.prop(prop_worker_context())
      .set(&Workspace::set_worker_context)
      .flags(peel::GObject::ParamFlags::CONSTRUCT_ONLY);
  }
};

} // namespace vin::module
