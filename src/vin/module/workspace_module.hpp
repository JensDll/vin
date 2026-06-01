#pragma once

#include "vin/lib/hyprland_workspace_list_model.hpp"
#include "vin/lib/iconfigurable.hpp"
#include "vin/lib/ipositionable.hpp"

#include <peel/GObject/Type.h>
#include <peel/class.h>
#include <peel/Gtk/Box.h>
#include <peel/Gtk/ListView.h>
#include <peel/Gtk/MenuButton.h>
#include <peel/Gtk/Orientable.h>
#include <peel/Gtk/Orientation.h>
#include <peel/Gtk/SignalListItemFactory.h>

extern "C" {
#include <lua.h>
}

namespace vin::module {

class WorkspaceModule final : public peel::Gtk::Box
{
private:
  PEEL_SIMPLE_CLASS(WorkspaceModule, Box)

  friend class peel::Gtk::Box;
  friend class lib::IPositionable;
  friend class lib::IConfigurable;

  peel::Gtk::ListView* m_normal_view;
  peel::Gtk::ListView* m_special_view;
  peel::Gtk::MenuButton* m_menu_button;

public:
  ~WorkspaceModule() = default;

  static auto create()
  {
    return peel::Object::create<WorkspaceModule>(
      peel::Gtk::Orientable::prop_orientation(), peel::Gtk::Orientation::HORIZONTAL, prop_spacing(), 0);
  }

private:
  void init(Class* cls);

  static void init_type(peel::Type type);

  static void init_interface(lib::IPositionable::Iface* iface);

  static void init_interface(lib::IConfigurable::Iface* iface);

  void on_setup(peel::Gtk::SignalListItemFactory* factory, peel::Object* obj);

  void on_bind_normal(peel::Gtk::SignalListItemFactory* factory, peel::Object* obj);

  void on_bind_special(peel::Gtk::SignalListItemFactory* factory, peel::Object* obj);

  void on_unbind(peel::Gtk::SignalListItemFactory* factory, peel::Object* obj);

  void on_no_special(lib::HyprlandWorkspaceListModel* model);

  void vfunc_position(lib::Position position);

  void vfunc_configure(lua_State* L);
};

} // namespace vin::module
