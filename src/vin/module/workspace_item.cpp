#include "vin/lib/hyprland_command.hpp"
#include "vin/lib/hyprland_workspace.hpp"
#include "vin/module/workspace_item.hpp"

#include <fmt/format.h>
#include <peel/class.h>
#include <peel/Gtk/Button.h>
#include <peel/Gtk/Label.h>
#include <peel/widget-template.h>
#include <spdlog/spdlog.h>

using namespace vin::module;
using namespace peel;

PEEL_CLASS_IMPL(WorkspaceItem, "VinModuleWorkspaceItem", Gtk::Button)

void WorkspaceItem::Class::init()
{
  spdlog::info("workspace class init");
  override_vfunc_dispose<WorkspaceItem>();
  set_template_from_resource("/com/doellmann/vin/workspace_item.ui");
  PEEL_WIDGET_TEMPLATE_BIND_CHILD(WorkspaceItem, m_label, "label");
}

void WorkspaceItem::init([[maybe_unused]] Class* const cls)
{
  spdlog::info("workspace item init");
  new (&m_on_clicked_command) decltype(&m_on_clicked_command);
  init_template();
  connect_clicked(this, &WorkspaceItem::on_clicked);
}

void WorkspaceItem::vfunc_dispose()
{
  spdlog::info("workspace item dispose");
  dispose_template(Type::of<WorkspaceItem>());
  m_workspace = nullptr;
  parent_vfunc_dispose<WorkspaceItem>();
}

void WorkspaceItem::on_clicked([[maybe_unused]] Gtk::Button* const button)
{
  UniquePtr<GLib::Error> error;
  const auto command{ vin::lib::HyprlandCommand::create(&error) };
  if (!command) {
    spdlog::error(error->message);
    return;
  }
  if (!command->send_and_forget(m_on_clicked_command, &error)) {
    spdlog::error(error->message);
  }
}

void WorkspaceItem::on_active_changed(Object* const workspace, [[maybe_unused]] peel::GObject::ParamSpec* const spec)
{
  if (workspace->cast<lib::HyprlandWorkspace>()->get_is_active()) {
    add_css_class("active");
  } else {
    remove_css_class("active");
  }
}

vin::lib::HyprlandWorkspace* WorkspaceItem::get_workspace()
{
  spdlog::info("get workspace");
  return m_workspace;
}

void WorkspaceItem::set_workspace(lib::HyprlandWorkspace* const workspace)
{
  spdlog::info("set workspace {}", fmt::ptr(workspace));

  g_return_if_fail(workspace == nullptr || workspace->check_type<lib::HyprlandWorkspace>());

  if (m_workspace == workspace) {
    return;
  }

  m_workspace = workspace;

  if (workspace != nullptr) {
    if (workspace->get_is_special()) {
      m_on_clicked_command =
        fmt::format("dispatch hl.dsp.workspace.toggle_special('{}')", workspace->get_special_name());
    } else {
      m_on_clicked_command = fmt::format("dispatch hl.dsp.focus({{workspace={}}})", workspace->get_id());
    }

    if (workspace->get_is_active()) {
      add_css_class("active");
    }

    workspace->connect_notify(lib::HyprlandWorkspace::prop_is_active(), this, &WorkspaceItem::on_active_changed);

    m_label->set_label(workspace->get_id_str());
  } else {
    set_child(nullptr);
  }

  notify(prop_workspace());
}
