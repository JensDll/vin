#pragma once

#include <peel/GObject/Object.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <peel/class.h>
#include <peel/property.h>
#include <peel/signal.h>

#include <limits>
#include <string>

namespace vin {

constexpr int WORKSPACE_MIN_ID{ -99 };
constexpr int WORKSPACE_MAX_ID{ std::numeric_limits<int>::max() };

class HyprlandWorkspace final : public peel::Object
{
  PEEL_SIMPLE_CLASS(HyprlandWorkspace, peel::Object)

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
  bool m_is_active;

public:
  ~HyprlandWorkspace() = default;

  static auto create(const nlohmann::json& json)
  {
    auto workspace{ peel::Object::create<HyprlandWorkspace>() };
    workspace->from_json(json);
    return workspace;
  }

  PEEL_PROPERTY(int, id, "id")

  [[nodiscard]] int get_id() const
  {
    return m_id;
  }

  PEEL_PROPERTY(const char*, id_str, "id-str");

  [[nodiscard]] const char* get_id_str() const
  {
    return m_id_str.c_str();
  }

  PEEL_PROPERTY(const char*, name, "name")

  [[nodiscard]] const char* get_name() const
  {
    return m_name.c_str();
  }

  PEEL_PROPERTY(const char*, special_name, "special-name")

  [[nodiscard]] const char* get_special_name() const
  {
    return m_name.c_str() + 8;
  }

  PEEL_PROPERTY(bool, is_special, "is-special")

  [[nodiscard]] bool get_is_special() const
  {
    return m_id < 0;
  }

  PEEL_PROPERTY(bool, is_normal, "is-normal")

  [[nodiscard]] bool get_is_normal() const
  {
    return !get_is_special();
  }

  PEEL_PROPERTY(bool, is_active, "is-active")

  [[nodiscard]] bool get_is_active() const
  {
    return m_is_active;
  }

  void set_is_active(const bool value)
  {
    m_is_active = value;
    notify(prop_is_active());
  }

private:
  void init(Class* cls);

  static void define_properties(auto& visitor)
  {
    visitor.prop(prop_id(), WORKSPACE_MIN_ID, WORKSPACE_MAX_ID, 0).get(&HyprlandWorkspace::get_id);
    visitor.prop(prop_id_str(), nullptr).get(&HyprlandWorkspace::get_id_str);
    visitor.prop(prop_name(), nullptr).get(&HyprlandWorkspace::get_name);
    visitor.prop(prop_special_name(), nullptr).get(&HyprlandWorkspace::get_special_name);
    visitor.prop(prop_is_special(), false).get(&HyprlandWorkspace::get_is_special);
    visitor.prop(prop_is_normal(), false).get(&HyprlandWorkspace::get_is_normal);
    visitor.prop(prop_is_active(), false).get(&HyprlandWorkspace::get_is_active).set(&HyprlandWorkspace::set_is_active);
  }

  void from_json(const nlohmann::json& json)
  {
    m_id_str = fmt::format("{}", json.at("id").get_to(m_id));
    json.at("name").get_to(m_name);
    json.at("monitor").get_to(m_monitor);
    json.at("monitorID").get_to(m_monitor_id);
    json.at("windows").get_to(m_windows);
    json.at("hasfullscreen").get_to(m_has_fullscreen);
    json.at("lastwindow").get_to(m_last_window);
    json.at("lastwindowtitle").get_to(m_last_window_title);
    json.at("ispersistent").get_to(m_is_persistent);
    json.at("tiledLayout").get_to(m_tiled_layout);
  }
};

} // namespace vin
