#include "vin/lib/hyprland_command.hpp"
#include "vin/lib/hyprland_connect.hpp"

#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <array>
#include <optional>

using namespace vin::lib;

HyprlandCommand::~HyprlandCommand()
{
  if (m_socket_fd != -1) {
    close(m_socket_fd);
  }
}

std::optional<HyprlandCommand> HyprlandCommand::create(peel::UniquePtr<peel::GLib::Error>* error)
{
  const auto socket_fd{ connect_hyprland<Connect::send>(error) };

  if (socket_fd == -1) {
    return {};
  }

  return HyprlandCommand(socket_fd);
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

std::optional<std::string> HyprlandCommand::send(const std::string& request,
  peel::UniquePtr<peel::GLib::Error>* error) const
{
  if (!send_and_forget(request, error)) {
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

bool HyprlandCommand::send_and_forget(const std::string& request, peel::UniquePtr<peel::GLib::Error>* error) const
{
  if (write_all(m_socket_fd, request) == -1) {
    peel::GLib::set_error(error,
      s_quark,
      static_cast<int>(Error::socket_failed_to_send_request),
      "Failed to send hyprland request '%s' : %s",
      request.c_str(),
      strerrordesc_np(errno));
    return false;
  }

  return true;
}
