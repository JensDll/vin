#pragma once

#include <peel/GObject/Object.h>
#include <nlohmann/json.hpp>
#include <peel/class.h>
#include <peel/property.h>
#include <peel/String.h>

#include <limits>
#include <string>

namespace vin::hyprland {

class WorkspaceButton final : public peel::Object
{
  PEEL_SIMPLE_CLASS(WorkspaceButton, peel::Object)

  std::string m_name;
  std::string m_monitor;
  std::string m_last_window;
  std::string m_last_window_title;
  std::string m_tiled_layout;
  std::string m_id_str;
  int m_id;
  unsigned int m_monitor_id;
  unsigned int m_windows;
  bool m_has_fullscreen;
  bool m_is_persistent;

public:
  ~WorkspaceButton() = default;

  static auto create(const nlohmann::json& json)
  {
    auto workspace{ peel::Object::create<WorkspaceButton>() };
    json.at("id").get_to(workspace->m_id);
    json.at("name").get_to(workspace->m_name);
    return workspace;
  }

  PEEL_PROPERTY(const char*, name, "name")
  PEEL_PROPERTY(const char*, special_name, "special-name")
  PEEL_PROPERTY(int, id, "id")

  [[nodiscard]] int get_id() const
  {
    return m_id;
  }

  [[nodiscard]] const char* get_name() const
  {
    return m_name.c_str();
  }

  [[nodiscard]] const char* get_special_name() const
  {
    return m_name.c_str() + 8;
  }

  [[nodiscard]] bool is_special() const
  {
    return m_id < 0;
  }

private:
  static void define_properties(auto& visitor)
  {
    visitor.prop(prop_id(), -99, std::numeric_limits<int>::max(), 0).get(&WorkspaceButton::get_id);
    visitor.prop(prop_name(), nullptr).get(&WorkspaceButton::get_name);
    visitor.prop(prop_special_name(), nullptr).get(&WorkspaceButton::get_special_name);
  }
};

} // namespace vin::hyprland
