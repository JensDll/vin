#pragma once

#include "mybar/wayland/surface.hpp"
#include "mybar/wayland/xdg_surface.hpp"
#include "xdg-shell-client-protocol.h"

#include <wayland-client.h>

#include <cstdint>

namespace wayland {

class xdg_wm_base;

struct xdg_wm_base_listener
{
  void (*ping)(void* data, xdg_wm_base* xdg_wm_base, uint32_t serial);
};

class xdg_wm_base
{
public:
  xdg_wm_base() = delete;
  xdg_wm_base(const xdg_wm_base&) = delete;
  xdg_wm_base(xdg_wm_base&&) = delete;
  xdg_wm_base& operator=(const xdg_wm_base&) = delete;
  xdg_wm_base& operator=(xdg_wm_base&&) = delete;
  ~xdg_wm_base() = delete;

  void add_listener(const xdg_wm_base_listener* const listener, void* const state)
  {
    ::xdg_wm_base_add_listener(
      reinterpret_cast<::xdg_wm_base*>(this), reinterpret_cast<const ::xdg_wm_base_listener*>(listener), state);
  }

  void pong(const uint32_t serial)
  {
    ::xdg_wm_base_pong(reinterpret_cast<::xdg_wm_base*>(this), serial);
  }

  xdg_surface* get_xdg_surface(surface* const surface)
  {
    return reinterpret_cast<xdg_surface*>(
      ::xdg_wm_base_get_xdg_surface(reinterpret_cast<::xdg_wm_base*>(this), reinterpret_cast<::wl_surface*>(surface)));
  }
};

} // namespace wayland
