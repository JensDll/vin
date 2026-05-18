#pragma once

#include "mybar/wayland/shm_pool.hpp"

namespace wayland {

class shm
{
public:
  shm() = delete;
  shm(const shm&) = delete;
  shm(shm&&) = delete;
  shm& operator=(const shm&) = delete;
  shm& operator=(shm&&) = delete;
  ~shm() = delete;

  shm_pool* create_pool(const int32_t fd, const int32_t size)
  {
    return reinterpret_cast<shm_pool*>(::wl_shm_create_pool(reinterpret_cast<::wl_shm*>(this), fd, size));
  }
};

} // namespace wayland
