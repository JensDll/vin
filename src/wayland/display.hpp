#pragma once

#include "mybar/wayland/registry.hpp"

#include <wayland-client.h>

#include <string>

namespace wayland {

class display
{
public:
  display() = delete;
  display(const display&) = delete;
  display(display&&) = delete;
  display& operator=(const display&) = delete;
  display& operator=(display&&) = delete;
  ~display() = delete;

  static display* connect()
  {
    return connect(nullptr);
  }

  static display* connect(const std::string& name)
  {
    return connect(name.c_str());
  }

  registry* get_registry()
  {
    return reinterpret_cast<registry*>(::wl_display_get_registry(reinterpret_cast<::wl_display*>(this)));
  }

  void roundtrip()
  {
    ::wl_display_roundtrip(reinterpret_cast<::wl_display*>(this));
  }

  bool dispatch()
  {
    return ::wl_display_dispatch(reinterpret_cast<::wl_display*>(this)) != 0;
  }

private:
  static display* connect(const char* name)
  {
    return reinterpret_cast<display*>(::wl_display_connect(name));
  }
};

} // namespace wayland
