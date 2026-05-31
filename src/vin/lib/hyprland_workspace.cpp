#include "vin/lib/hyprland_workspace.hpp"

#include <peel/class.h>
#include <spdlog/spdlog.h>

using namespace peel;
using namespace vin::lib;

PEEL_CLASS_IMPL(HyprlandWorkspace, "VinHyprlandWorkspace", Object)

void HyprlandWorkspace::init([[maybe_unused]] Class* const cls)
{
  new (&m_signal_connections) decltype(m_signal_connections);
  new (&m_name) decltype(m_name);
  new (&m_monitor) decltype(m_monitor);
  new (&m_last_window) decltype(m_last_window);
  new (&m_last_window_title) decltype(m_last_window_title);
  new (&m_tiled_layout) decltype(m_tiled_layout);
  new (&m_id_str) decltype(m_id_str);
}

void HyprlandWorkspace::Class::init() {}
