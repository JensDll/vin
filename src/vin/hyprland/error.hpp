#pragma once

#include <peel/GLib/Quark.h>

namespace vin::hyprland {

enum class Error : unsigned char {
  env_no_xdg_runtime_dir,
  env_no_hyprland_instance_signature,
  socket_path_too_long,
  socket_failed_to_create,
  socket_failed_to_connect,
  socket_failed_to_send_request,
  socket_failed_to_read_response
};

const static peel::GLib::Quark s_quark{ peel::GLib::Quark("vin-hyprland") };

} // namespace vin::hyprland
