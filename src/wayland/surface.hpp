#pragma once

#include "mybar/wayland/buffer.hpp"
#include "mybar/wayland/callback.hpp"
#include "mybar/wayland/compositor.hpp"

#include <wayland-client-protocol.h>
#include <wayland-client.h>

#include <cstdint>

namespace wayland {

class surface
{
public:
  surface() = delete;
  surface(const surface&) = delete;
  surface(surface&&) = delete;
  surface& operator=(const surface&) = delete;
  surface& operator=(surface&&) = delete;
  ~surface() = delete;

  static surface* create(compositor* const compositor)
  {
    return reinterpret_cast<surface*>(::wl_compositor_create_surface(reinterpret_cast<::wl_compositor*>(compositor)));
  }

  void attach(wayland::buffer* const buffer, const int32_t x, const int32_t y)
  {
    ::wl_surface_attach(reinterpret_cast<::wl_surface*>(this), reinterpret_cast<::wl_buffer*>(buffer), x, y);
  }

  void commit()
  {
    ::wl_surface_commit(reinterpret_cast<::wl_surface*>(this));
  }

  callback* frame()
  {
    return reinterpret_cast<callback*>(::wl_surface_frame(reinterpret_cast<::wl_surface*>(this)));
  }

  void damage_buffer(const int32_t x, const int32_t y, const int32_t width, const int32_t height)
  {
    ::wl_surface_damage_buffer(reinterpret_cast<::wl_surface*>(this), x, y, width, height);
  }
};

} // namespace wayland
