#include "vin/hyprland/command.hpp"
#include "vin/hyprland/connect.hpp"

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <array>
#include <optional>

using namespace vin::hyprland;

Command::~Command()
{
  if (m_socket_fd != -1) {
    close(m_socket_fd);
  }
}

std::optional<Command> Command::create(peel::UniquePtr<peel::GLib::Error>* error)
{
  const auto socket_fd{ connect<Connect::send>(error) };

  if (socket_fd == -1) {
    return {};
  }

  return Command(socket_fd);
}

namespace {

auto write_all(const int fd, const auto& buffer)
{
  const auto* data{ buffer.data() };
  auto count{ buffer.size() };
  auto num_written{ write(fd, data, count) };
  while (num_written != -1 && (count -= num_written) != 0) [[unlikely]] {
    num_written = write(fd, data += num_written, count);
  }
  return num_written;
}

auto read_all(const int fd, std::string& output)
{
  std::array<char, 4096> buffer;
  ssize_t num_read; // NOLINT(cppcoreguidelines-init-variables)
  do {
    num_read = read(fd, buffer.data(), buffer.size());
    output.append(buffer.data(), num_read);
  } while (num_read > 0);
  return num_read;
}

} // namespace

std::optional<std::string> Command::send(const std::string& request, peel::UniquePtr<peel::GLib::Error>* error) const
{
  if (write_all(m_socket_fd, request) == -1) {
    peel::GLib::set_error(error,
      s_quark,
      static_cast<int>(Error::socket_failed_to_send_request),
      "Failed to send hyprland request '%s' : %s",
      request.c_str(),
      strerrordesc_np(errno));
    return {};
  }

  std::string response;

  if (read_all(m_socket_fd, response) == -1) {
    peel::GLib::set_error(error,
      s_quark,
      static_cast<int>(Error::socket_failed_to_read_response),
      "Failed to read hyprland response for request '%s' : %s",
      request.c_str(),
      strerrordesc_np(errno));
    return {};
  }

  return response;
}

void Command::send_and_forget(const std::string& request, peel::UniquePtr<peel::GLib::Error>* error) const
{
  if (write_all(m_socket_fd, request) == -1) {
    peel::GLib::set_error(error,
      s_quark,
      static_cast<int>(Error::socket_failed_to_send_request),
      "Failed to send hyprland request '%s' : %s",
      request.c_str(),
      strerrordesc_np(errno));
  }
}
