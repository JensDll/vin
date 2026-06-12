#pragma once

#include "vin/lib/hyprland_event_listener.hpp"
#include "vin/lib/hyprland_workspace.hpp"

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

namespace vin::lib {

class HyprlandWorkspaceListModel final : public peel::Gio::ListModel
{
  PEEL_SIMPLE_CLASS(HyprlandWorkspaceListModel, peel::Gio::ListModel)

  friend class peel::Gio::ListModel;
  friend class peel::Gio::Initable;

  using SignalNewSpecial = peel::Signal<HyprlandWorkspaceListModel, void(std::size_t)>;
  using SignalRemoveSpecial = peel::Signal<HyprlandWorkspaceListModel, void(std::size_t)>;

  static SignalRemoveSpecial s_signal_new_special;
  static SignalRemoveSpecial s_signal_remove_special;

  std::map<int, peel::RefPtr<HyprlandWorkspace>> m_items;

  peel::GLib::MainContext* m_main_context;
  peel::GLib::MainContext* m_worker_context;

  peel::RefPtr<HyprlandEventListener> m_event_listener;

  std::size_t m_num_special;
  int m_active_id;
  int m_active_special_id;

public:
  static auto create(peel::Gio::Cancellable* const cancellable,
    peel::UniquePtr<peel::GLib::Error>* const error,
    peel::GLib::MainContext* const main_context,
    peel::GLib::MainContext* const worker_context)
  {
    return vin::gio::initable_create<HyprlandWorkspaceListModel>(
      cancellable, error, prop_main_context(), main_context, prop_worker_context(), worker_context);
  }

  PEEL_SIGNAL_CONNECT_METHOD(new_special, s_signal_new_special);
  PEEL_SIGNAL_CONNECT_METHOD(remove_special, s_signal_remove_special);

  [[nodiscard]] std::size_t num_special() const
  {
    return m_num_special;
  }

private:
  static void init_type(peel::Type type);

  static void init_interface(peel::Gio::Initable::Iface* iface);

  static void init_interface(peel::Gio::ListModel::Iface* iface);

  bool vfunc_init(peel::Gio::Cancellable* cancellable, peel::UniquePtr<peel::GLib::Error>* error);

  void vfunc_dispose();

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
      .set(&HyprlandWorkspaceListModel::set_main_context)
      .flags(peel::GObject::ParamFlags::CONSTRUCT_ONLY);
    visitor.prop(prop_worker_context())
      .set(&HyprlandWorkspaceListModel::set_worker_context)
      .flags(peel::GObject::ParamFlags::CONSTRUCT_ONLY);
  }
};

} // namespace vin::lib
