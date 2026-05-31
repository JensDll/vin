#pragma once

#include <peel/GLib/Quark.h>

namespace vin {

enum class Error : unsigned char {
  failed_to_determine_home,
  css_monitor_failed,
  config_monitor_failed,
  failed_to_load_config,
  failed_to_run_config
};

const static peel::GLib::Quark s_quark{ peel::GLib::Quark("vin") };

} // namespace vin
