#pragma once

#include <peel/GLib/Quark.h>

namespace vin::lib {

enum class Error : unsigned char {
  failed_to_determine_xdg_runtime_dir,
  failed_to_determine_hyprland_instance_signature,
  socket_path_too_long,
  socket_failed_to_create,
  socket_failed_to_connect,
  socket_failed_to_send_request,
  socket_failed_to_read_response
};

const static peel::GLib::Quark s_quark{ peel::GLib::Quark("vin-lib") };

} // namespace vin::lib
