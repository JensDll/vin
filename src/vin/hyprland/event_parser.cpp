#include "vin/hyprland/event_parser.hpp"

#include <charconv>
#include <cstring>

int vin::hyprland::parse_workspace_id(const char* const data)
{
  int result{};
  std::from_chars(data, std::strchr(data, ','), result);
  return result;
}
