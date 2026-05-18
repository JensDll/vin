#pragma once

#include "mybar/wayland/xdg_toplevel.hpp"
#include "xdg-shell-client-protocol.h"

#include <cstdint>

namespace wayland {

class xdg_surface;

struct xdg_surface_listener
{
  void (*configure)(void* data, xdg_surface* xdg_surface, uint32_t serial);
};

class xdg_surface
{
public:
  xdg_surface() = delete;
  xdg_surface(const xdg_surface&) = delete;
  xdg_surface(xdg_surface&&) = delete;
  xdg_surface& operator=(const xdg_surface&) = delete;
  xdg_surface& operator=(xdg_surface&&) = delete;
  ~xdg_surface() = delete;

  void add_listener(const xdg_surface_listener* const listsner, void* const data)
  {
    ::xdg_surface_add_listener(
      reinterpret_cast<::xdg_surface*>(this), reinterpret_cast<const ::xdg_surface_listener*>(listsner), data);
  }

  void ack_configure(const uint32_t serial)
  {
    ::xdg_surface_ack_configure(reinterpret_cast<::xdg_surface*>(this), serial);
  }

  xdg_toplevel* get_toplevel()
  {
    return reinterpret_cast<xdg_toplevel*>(::xdg_surface_get_toplevel(reinterpret_cast<::xdg_surface*>(this)));
  }
};

} // namespace wayland
