#pragma once

#include "vin/hyprland/event_listener.hpp"
#include "vin/hyprland/workspace_button.hpp"

#include <peel/GObject/Object.h>
#include <nlohmann/json.hpp>
#include <peel/class.h>
#include <peel/FloatPtr.h>
#include <peel/Gio/ListStore.h>
#include <peel/Gtk/Box.h>
#include <peel/Gtk/Button.h>
#include <peel/Gtk/DropDown.h>
#include <peel/Gtk/Orientation.h>
#include <peel/Gtk/Widget.h>
#include <peel/RefPtr.h>
#include <peel/signal.h>

#include <map>

namespace vin::hyprland {

class Workspaces final : public peel::Gtk::Box
{
  PEEL_SIMPLE_CLASS(Workspaces, Box)

  friend class peel::Gtk::Box;

  struct MapEntry
  {
    WorkspaceButton* button;
    peel::SignalConnection click_connection;
  };

  std::map<int, MapEntry> m_workspaces;
  peel::Gtk::DropDown* m_dropdown;
  peel::Gio::ListStore* m_dropdown_items;
  int m_active_id;
  int m_active_special_id;

public:
  ~Workspaces() = default;

  static auto create(const peel::Gtk::Orientation orientation, const int spacing)
  {
    return peel::Object::create<Workspaces>(
      peel::Property<peel::Gtk::Orientation>("orientation"), orientation, prop_spacing(), spacing);
  }

private:
  void init(Class* cls);

  void on_workspace_change(EventListener* event_listener, const char* data);

  void on_new_workspace(EventListener* event_listener, const char* data);

  void on_remove_workspace(EventListener* event_listener, const char* data);

  void on_active_special(EventListener* event_listener, const char* data);

  void vfunc_dispose();

  decltype(m_workspaces)::const_iterator insert_workspace(const nlohmann::json& json);
};

} // namespace vin::hyprland
