#pragma once

#include <wayland-client-protocol.h>

namespace wayland {

class callback;

struct callback_listener
{
  void (*done)(void* data, callback* callback, uint32_t time);
};

class callback
{
public:
  callback() = delete;
  callback(const callback&) = delete;
  callback(callback&&) = delete;
  callback& operator=(const callback&) = delete;
  callback& operator=(callback&&) = delete;
  ~callback() = delete;

  void add_listener(const callback_listener* const listener, void* const data)
  {
    ::wl_callback_add_listener(
      reinterpret_cast<::wl_callback*>(this), reinterpret_cast<const ::wl_callback_listener*>(listener), data);
  }

  void destroy()
  {
    ::wl_callback_destroy(reinterpret_cast<::wl_callback*>(this));
  }
};

} // namespace wayland
