#pragma once

#include <peel/GLib/Error.h>
#include <peel/UniquePtr.h>

#include <optional>
#include <string>
#include <utility>

namespace vin::hyprland {

class Command
{
private:
  int m_socket_fd{ -1 };

  Command(const int socket_fd) noexcept
    : m_socket_fd(socket_fd)
  {}

public:
  static std::optional<Command> create(peel::UniquePtr<peel::GLib::Error>* error);

  Command(const Command&) = delete;

  Command(Command&& other) noexcept
  {
    using std::swap;
    swap(*this, other);
  }

  ~Command();

  Command& operator=(const Command&) = delete;

  Command& operator=(Command&& other) noexcept
  {
    using std::swap;
    swap(*this, other);
    return *this;
  }

  [[nodiscard]] std::optional<std::string> send(const std::string& request,
    peel::UniquePtr<peel::GLib::Error>* error) const;

  void send_and_forget(const std::string& request, peel::UniquePtr<peel::GLib::Error>* error) const;

  friend void swap(Command& a, Command& b) noexcept;
};

inline void swap(Command& a, Command& b) noexcept
{
  using std::swap;
  swap(a.m_socket_fd, b.m_socket_fd);
}

} // namespace vin::hyprland
