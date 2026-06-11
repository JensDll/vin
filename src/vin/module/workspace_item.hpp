#pragma once

#include "vin/lib/hyprland_workspace.hpp"

#include <peel/GObject/Object.h>
#include <peel/GObject/ParamSpec.h>
#include <peel/class.h>
#include <peel/Gtk/Button.h>
#include <peel/Gtk/Label.h>
#include <peel/Gtk/Widget.h>
#include <peel/property.h>
#include <peel/RefPtr.h>
#include <spdlog/spdlog.h>

namespace vin::module {

class WorkspaceItem final : public peel::Gtk::Button
{
  PEEL_SIMPLE_CLASS(WorkspaceItem, peel::Gtk::Button)

  std::string m_on_clicked_command;
  peel::RefPtr<lib::HyprlandWorkspace> m_workspace;
  peel::Gtk::Label* m_label;

public:
  ~WorkspaceItem() = default;

  lib::HyprlandWorkspace* get_workspace();

  void set_workspace(lib::HyprlandWorkspace* workspace);

  PEEL_PROPERTY(lib::HyprlandWorkspace, workspace, "workspace");

private:
  void init(Class* cls);

  void vfunc_dispose();

  void on_clicked(peel::Gtk::Button* button);

  void on_active_changed(peel::Object* workspace, peel::GObject::ParamSpec* spec);

  static void define_properties(auto& visitor)
  {
    visitor.prop(prop_workspace()).get(&WorkspaceItem::get_workspace).set(&WorkspaceItem::set_workspace);
  }
};

} // namespace vin::module
