#pragma once

namespace wayland {

class compositor
{
public:
  compositor() = delete;
  compositor(const compositor&) = delete;
  compositor(compositor&&) = delete;
  compositor& operator=(const compositor&) = delete;
  compositor& operator=(compositor&&) = delete;
  ~compositor() = delete;
};

} // namespace wayland
