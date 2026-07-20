#pragma once

#include "vin/hyprland_event_listener.hpp"
#include "vin/hyprland_workspace.hpp"
#include "vin/main_context.hpp"

#include <peel/class.h>
#include <peel/Gio/ListModel.h>
#include <peel/GLib/MainContext.h>
#include <peel/property.h>
#include <peel/RefPtr.h>
#include <peel/signal.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <iterator>
#include <map>

namespace vin {

class HyprlandWorkspaceListModel final : public peel::Gio::ListModel
{
  PEEL_SIMPLE_CLASS(HyprlandWorkspaceListModel, peel::Gio::ListModel)

  friend class peel::Gio::Initable;

  std::map<int, peel::RefPtr<HyprlandWorkspace>> m_items;

  peel::GLib::MainContext* m_main_context;
  peel::GLib::MainContext* m_worker_context;

  peel::RefPtr<HyprlandEventListener> m_event_listener;

  std::size_t m_num_special;
  int m_active_id;
  int m_active_special_id;

public:
  ~HyprlandWorkspaceListModel() = default;

  static peel::Type vfunc_get_item_type()
  {
    return peel::Type::of<HyprlandWorkspaceListModel>();
  }

  [[nodiscard]] unsigned int vfunc_get_n_items() const
  {
    return m_items.size();
  }

  [[nodiscard]] peel::RefPtr<peel::Object> vfunc_get_item(const unsigned int pos) const
  {
    return pos < vfunc_get_n_items() ? std::next(m_items.begin(), pos)->second : nullptr;
  }

  VIN_MAIN_CONTEXT_PROPERTY

  PEEL_PROPERTY(bool, is_any_special, "is-any-special")
  [[nodiscard]] bool get_is_any_special() const;

private:
  bool vfunc_init(peel::Gio::Cancellable* cancellable, peel::UniquePtr<peel::GLib::Error>* error);

  void vfunc_dispose();

  void on_workspace_change(HyprlandEventListener* event_listener, const char* data);

  void on_new_workspace(HyprlandEventListener* event_listener, const char* data);

  void on_remove_workspace(HyprlandEventListener* event_listener, const char* data);

  void on_active_special(HyprlandEventListener* event_listener, const char* data);

  static void init_type(peel::Type type);

  static void init_interface(peel::Gio::Initable::Iface* iface);

  static void init_interface(peel::Gio::ListModel::Iface* iface);

  static void define_properties(auto& visitor);
};

} // namespace vin
