#pragma once

#include "vin/lib/error.hpp"

#include <fmt/format.h>
#include <peel/GLib/Error.h>
#include <peel/GLib/functions.h>
#include <peel/GLib/Quark.h>
#include <peel/UniquePtr.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace vin::lib {

enum class Connect : unsigned char { send, receive };

template<Connect C>
consteval auto connect_hyprland_path()
{
  if constexpr (C == Connect::send) {
    return "{}/hypr/{}/.socket.sock";
  }

  if constexpr (C == Connect::receive) {
    return "{}/hypr/{}/.socket2.sock";
  }
}

template<Connect C>
int connect_hyprland(peel::UniquePtr<peel::GLib::Error>* const error)
{
  const char* const runtime_dir{ std::getenv("XDG_RUNTIME_DIR") };

  if (runtime_dir == nullptr) {
    peel::GLib::set_error_literal(
      error, s_quark, int(Error::failed_to_determine_xdg_runtime_dir), "failed to determine XDG_RUNTIME_DIR");
    return -1;
  }

  const char* const his{ std::getenv("HYPRLAND_INSTANCE_SIGNATURE") };

  if (his == nullptr) {
    peel::GLib::set_error_literal(error,
      s_quark,
      int(Error::failed_to_determine_hyprland_instance_signature),
      "failed to determine HYPRLAND_INSTANCE_SIGNATURE");
    return -1;
  }

  sockaddr_un addr;
  addr.sun_family = AF_UNIX;

  const auto result{ fmt::format_to_n(
    static_cast<char*>(addr.sun_path), sizeof(addr.sun_path), connect_hyprland_path<C>(), runtime_dir, his) };

  if (result.size >= sizeof(addr.sun_path)) {
    peel::GLib::set_error_literal(
      error, s_quark, static_cast<int>(Error::socket_path_too_long), "path to hyprland socket is too long");
    return -1;
  }

  *result.out = '\0';

  const int socket_fd{ socket(AF_UNIX, SOCK_STREAM, 0) };

  if (socket_fd == -1) {
    peel::GLib::set_error(error,
      s_quark,
      static_cast<int>(Error::socket_failed_to_create),
      "failed to create socket : %s",
      strerrordesc_np(errno));
    return -1;
  }

  if (::connect(socket_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
    peel::GLib::set_error(error,
      s_quark,
      static_cast<int>(Error::socket_failed_to_connect),
      "failed to connect to %s : %s",
      static_cast<const char*>(addr.sun_path),
      strerrordesc_np(errno));
    close(socket_fd);
    return -1;
  }

  return socket_fd;
}

} // namespace vin::lib
