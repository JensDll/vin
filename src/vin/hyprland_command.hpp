#pragma once

#include <peel/GLib/Error.h>
#include <peel/UniquePtr.h>

#include <optional>
#include <string>
#include <utility>

namespace vin {

class HyprlandCommand
{
private:
  int m_socket_fd{ -1 };

  HyprlandCommand(const int socket_fd) noexcept
    : m_socket_fd(socket_fd)
  {}

public:
  static std::optional<HyprlandCommand> create(peel::UniquePtr<peel::GLib::Error>* error);

  HyprlandCommand(const HyprlandCommand&) = delete;

  HyprlandCommand(HyprlandCommand&& other) noexcept
  {
    using std::swap;
    swap(*this, other);
  }

  ~HyprlandCommand();

  HyprlandCommand& operator=(const HyprlandCommand&) = delete;

  HyprlandCommand& operator=(HyprlandCommand&& other) noexcept
  {
    using std::swap;
    swap(*this, other);
    return *this;
  }

  [[nodiscard]] std::optional<std::string> send(const std::string& request,
    peel::UniquePtr<peel::GLib::Error>* error) const;

  bool send_and_forget(const std::string& request, peel::UniquePtr<peel::GLib::Error>* error) const;

  friend void swap(HyprlandCommand& a, HyprlandCommand& b) noexcept;
};

inline void swap(HyprlandCommand& a, HyprlandCommand& b) noexcept
{
  using std::swap;
  swap(a.m_socket_fd, b.m_socket_fd);
}

} // namespace vin
