#include "vin/hyprland_command.hpp"
#include "vin/hyprland_workspace.hpp"
#include "vin/hyprland_workspace_item.hpp"

#include <fmt/format.h>
#include <peel/class.h>
#include <peel/Gtk/Button.h>
#include <peel/Gtk/Label.h>
#include <peel/widget-template.h>
#include <spdlog/spdlog.h>

using namespace vin;
using namespace peel;

PEEL_CLASS_IMPL(HyprlandWorkspaceItem, "VinHyprlandWorkspaceItem", Gtk::Button)

void HyprlandWorkspaceItem::Class::init()
{
  override_vfunc_dispose<HyprlandWorkspaceItem>();
  set_template_from_resource("/com/doellmann/vin/hyprland_workspace_item.ui");
  PEEL_WIDGET_TEMPLATE_BIND_CHILD(HyprlandWorkspaceItem, m_label, "label");
  PEEL_WIDGET_TEMPLATE_BIND_CALLBACK(HyprlandWorkspaceItem, on_clicked, "on_clicked");
}

void HyprlandWorkspaceItem::init([[maybe_unused]] Class* const cls)
{
  new (&m_on_clicked_command) decltype(&m_on_clicked_command);
  init_template();
}

void HyprlandWorkspaceItem::vfunc_dispose()
{
  spdlog::info("[VinHyprlandWorkspaceItem] dispose");
  m_workspace = nullptr;
  dispose_template(Type::of<HyprlandWorkspaceItem>());
  parent_vfunc_dispose<HyprlandWorkspaceItem>();
}

void HyprlandWorkspaceItem::on_clicked([[maybe_unused]] Gtk::Button* const button)
{
  UniquePtr<GLib::Error> error;
  const auto command{ HyprlandCommand::create(&error) };
  if (!command) {
    spdlog::error(error->message);
    return;
  }
  if (!command->send_and_forget(m_on_clicked_command, &error)) {
    spdlog::error(error->message);
  }
}

void HyprlandWorkspaceItem::on_active_changed(Object* const workspace,
  [[maybe_unused]] peel::GObject::ParamSpec* const spec)
{
  if (workspace->cast<HyprlandWorkspace>()->get_is_active()) {
    add_css_class("active");
  } else {
    remove_css_class("active");
  }
}

void HyprlandWorkspaceItem::set_workspace(HyprlandWorkspace* const workspace)
{
  g_return_if_fail(workspace == nullptr || workspace->check_type<HyprlandWorkspace>());

  if (m_workspace == workspace) {
    return;
  }

  m_workspace = workspace;

  if (workspace != nullptr) {
    if (workspace->get_is_special()) {
      m_on_clicked_command =
        fmt::format("dispatch hl.dsp.workspace.toggle_special('{}')", workspace->get_special_name());
      m_label->set_label(workspace->get_special_name());
    } else {
      m_on_clicked_command = fmt::format("dispatch hl.dsp.focus({{workspace={}}})", workspace->get_id_str());
      m_label->set_label(workspace->get_id_str());
    }

    if (workspace->get_is_active()) {
      add_css_class("active");
    }

    workspace->connect_notify(HyprlandWorkspace::prop_is_active(), this, &HyprlandWorkspaceItem::on_active_changed);
  } else {
    set_child(nullptr);
  }

  notify(prop_workspace());
}

void HyprlandWorkspaceItem::define_properties(auto& visitor)
{
  visitor.prop(prop_workspace()).set(&HyprlandWorkspaceItem::set_workspace);
}
