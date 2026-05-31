#include <fmt/base.h>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/std.h>

#include <array>
#include <chrono>

int main()
{
  const auto now{ std::chrono::system_clock::now() };
  const auto* const time_zone{ std::chrono::current_zone() };

  std::array<char, 5> buffer;

  const auto result = fmt::format_to_n(buffer.data(), buffer.size(), "1234567");

  fmt::println("{} {} {}", fmt::ptr(buffer.data() + buffer.size()), fmt::ptr(result.out), result.size);
}
