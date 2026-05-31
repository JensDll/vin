#pragma once

#include "vin/lib/hyprland_event_listener.hpp"
#include "vin/lib/hyprland_workspace.hpp"

#include <peel/class.h>
#include <peel/Gio/ListModel.h>
#include <peel/Gio/ListStore.h>
#include <peel/property.h>
#include <peel/RefPtr.h>
#include <peel/signal.h>
#include <spdlog/spdlog.h>

#include <cstddef>
#include <iterator>
#include <map>

namespace vin::lib {

class HyprlandWorkspaceListModel final : public peel::Gio::ListModel
{
  PEEL_SIMPLE_CLASS(HyprlandWorkspaceListModel, peel::Gio::ListModel)

  friend class peel::Gio::ListModel;

  using SignalNoSpecial = peel::Signal<HyprlandWorkspaceListModel, void()>;

  static SignalNoSpecial s_signal_no_special;

  std::map<int, peel::RefPtr<HyprlandWorkspace>> m_items;
  std::size_t m_num_special;
  int m_active_id;
  int m_active_special_id;

public:
  ~HyprlandWorkspaceListModel() = default;

  static auto create()
  {
    return peel::Object::create<HyprlandWorkspaceListModel>();
  }

  PEEL_SIGNAL_CONNECT_METHOD(no_special, s_signal_no_special);

  [[nodiscard]] std::size_t num_special() const
  {
    return m_num_special;
  }

private:
  void init(Class* cls);

  static void init_type(const peel::Type type)
  {
    PEEL_IMPLEMENT_INTERFACE(type, peel::Gio::ListModel);
  }

  static void init_interface(peel::Gio::ListModel::Iface* const iface)
  {
    iface->override_vfunc_get_item<HyprlandWorkspaceListModel>();
    iface->override_vfunc_get_item_type<HyprlandWorkspaceListModel>();
    iface->override_vfunc_get_n_items<HyprlandWorkspaceListModel>();
  }

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

  void on_workspace_change(HyprlandEventListener* event_listener, const char* data);

  void on_new_workspace(HyprlandEventListener* event_listener, const char* data);

  void on_remove_workspace(HyprlandEventListener* event_listener, const char* data);

  void on_active_special(HyprlandEventListener* event_listener, const char* data);
};

} // namespace vin::lib
