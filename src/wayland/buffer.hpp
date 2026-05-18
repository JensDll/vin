#pragma once

#include <wayland-client-protocol.h>
namespace wayland {

class buffer;

struct buffer_listener
{
  void (*release)(void* data, buffer* buffer);
};

class buffer
{
public:
  buffer() = delete;
  buffer(const buffer&) = delete;
  buffer(buffer&&) = delete;
  buffer& operator=(const buffer&) = delete;
  buffer& operator=(buffer&&) = delete;
  ~buffer() = delete;

  void add_listener(const buffer_listener* const listener, void* const data)
  {
    ::wl_buffer_add_listener(
      reinterpret_cast<::wl_buffer*>(this), reinterpret_cast<const ::wl_buffer_listener*>(listener), data);
  }

  void destroy()
  {
    ::wl_buffer_destroy(reinterpret_cast<::wl_buffer*>(this));
  }
};

} // namespace wayland
