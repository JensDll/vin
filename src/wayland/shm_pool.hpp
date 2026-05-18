#pragma once

#include "mybar/wayland/buffer.hpp"

#include <wayland-client-protocol.h>

#include <cstdint>

namespace wayland {

class shm_pool
{
public:
  shm_pool() = delete;
  shm_pool(const shm_pool&) = delete;
  shm_pool(shm_pool&&) = delete;
  shm_pool& operator=(const shm_pool&) = delete;
  shm_pool& operator=(shm_pool&&) = delete;
  ~shm_pool() = delete;

  buffer* create_buffer(const int32_t offset,
    const int32_t width,
    const int32_t height,
    const int32_t stride,
    const uint32_t format)
  {
    return reinterpret_cast<buffer*>(
      ::wl_shm_pool_create_buffer(reinterpret_cast<::wl_shm_pool*>(this), offset, width, height, stride, format));
  }

  void destroy()
  {
    ::wl_shm_pool_destroy(reinterpret_cast<::wl_shm_pool*>(this));
  }
};

} // namespace wayland
