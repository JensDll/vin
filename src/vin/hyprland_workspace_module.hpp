#pragma once

#include "vin/hyprland_workspace_list_model.hpp"
#include "vin/iconfigurable.hpp"
#include "vin/ipositionable.hpp"
#include "vin/main_context.hpp"

#include <peel/GObject/Object.h>
#include <peel/class.h>
#include <peel/Gio/Initable.h>
#include <peel/GLib/MainContext.h>
#include <peel/Gtk/Box.h>
#include <peel/Gtk/ListView.h>
#include <peel/Gtk/MenuButton.h>

namespace vin {

class HyprlandWorkspaceModule final : public peel::Gtk::Box
{
  PEEL_SIMPLE_CLASS(HyprlandWorkspaceModule, peel::Gtk::Box)

  friend class IPositionable;
  friend class IConfigurable;
  friend class peel::Gio::Initable;

  peel::GLib::MainContext* m_main_context;
  peel::GLib::MainContext* m_worker_context;
  HyprlandWorkspaceListModel* m_model;
  peel::Gtk::ListView* m_normal_view;
  peel::Gtk::MenuButton* m_menu_button;

public:
  ~HyprlandWorkspaceModule() = default;

  static auto create(const MainContext context)
  {
    return peel::Object::create<HyprlandWorkspaceModule>(
      prop_main_context(), context.main_context, prop_worker_context(), context.worker_context);
  }

  VIN_MAIN_CONTEXT_PROPERTY

private:
  void init(Class* cls);

  static void init_type(peel::Type type);

  static void init_interface(IPositionable::Iface* iface);

  static void init_interface(IConfigurable::Iface* iface);

  static void init_interface(peel::Gio::Initable::Iface* iface);

  bool vfunc_init(peel::Gio::Cancellable* cancellable, peel::UniquePtr<peel::GLib::Error>* error);

  void vfunc_dispose();

  void vfunc_position(Position position);

  void vfunc_configure(lua_State* L);

  static void define_properties(auto& visitor);
};

} // namespace vin
