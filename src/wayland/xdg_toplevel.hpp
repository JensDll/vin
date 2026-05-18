#pragma once

#include "xdg-shell-client-protocol.h"

namespace wayland {

class xdg_toplevel
{
public:
  xdg_toplevel() = delete;
  xdg_toplevel(const xdg_toplevel&) = delete;
  xdg_toplevel(xdg_toplevel&&) = delete;
  xdg_toplevel& operator=(const xdg_toplevel&) = delete;
  xdg_toplevel& operator=(xdg_toplevel&&) = delete;
  ~xdg_toplevel() = delete;

  void set_title(const char* const title)
  {
    ::xdg_toplevel_set_title(reinterpret_cast<::xdg_toplevel*>(this), title);
  }
};

} // namespace wayland
