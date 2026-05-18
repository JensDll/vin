#include <fmt/base.h>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>

#include <vector>

namespace {

class Workspace
{
private:
  std::string m_name;
  std::string m_monitor;
  std::string m_last_window;
  std::string m_last_window_title;
  std::string m_tiled_layout;
  std::string m_id_str;
  unsigned int m_id;
  unsigned int m_monitor_id;
  unsigned int m_windows;
  bool m_has_fullscreen;
  bool m_is_persistent;

  friend void from_json(const nlohmann::json& json, Workspace& workspace);

public:
  [[nodiscard]] const auto& name() const
  {
    return m_name;
  }

  [[nodiscard]] const auto& monitor() const
  {
    return m_monitor;
  }

  [[nodiscard]] const auto& last_window() const
  {
    return m_last_window;
  }

  [[nodiscard]] const auto& last_window_title() const
  {
    return m_last_window_title;
  }

  [[nodiscard]] const auto& tiled_layout() const
  {
    return m_tiled_layout;
  }

  [[nodiscard]] auto id() const
  {
    return m_id;
  }

  [[nodiscard]] auto& id_str() const
  {
    return m_id_str;
  }

  [[nodiscard]] auto monitor_id() const
  {
    return m_monitor_id;
  }

  [[nodiscard]] auto num_windows() const
  {
    return m_windows;
  }

  [[nodiscard]] auto has_fullscreen() const
  {
    return m_has_fullscreen;
  }

  [[nodiscard]] auto is_persistent() const
  {
    return m_is_persistent;
  }

  bool operator<(const Workspace& other) const
  {
    return m_id < other.m_id;
  }
};

void from_json(const nlohmann::json& json, Workspace& workspace)
{
  json.at("id").get_to(workspace.m_id);
  json.at("name").get_to(workspace.m_name);
  json.at("monitor").get_to(workspace.m_monitor);
  json.at("monitorID").get_to(workspace.m_monitor_id);
  json.at("windows").get_to(workspace.m_windows);
  json.at("hasfullscreen").get_to(workspace.m_has_fullscreen);
  json.at("lastwindow").get_to(workspace.m_last_window);
  json.at("lastwindowtitle").get_to(workspace.m_last_window_title);
  json.at("ispersistent").get_to(workspace.m_is_persistent);
  json.at("tiledLayout").get_to(workspace.m_tiled_layout);
  workspace.m_id_str = fmt::format("{}", workspace.m_id);
}

} // namespace

int main()
{
  const std::string input{ R"([{
    "id": 1,
    "name": "1",
    "monitor": "DP-1",
    "monitorID": 0,
    "windows": 1,
    "hasfullscreen": false,
    "lastwindow": "0x5601df560610",
    "lastwindowtitle": "./out/build/debug/src/vin/vin ",
    "ispersistent": false,
    "tiledLayout": "dwindle"
},{
    "id": 2,
    "name": "2",
    "monitor": "DP-1",
    "monitorID": 0,
    "windows": 1,
    "hasfullscreen": false,
    "lastwindow": "0x5601df57c9b0",
    "lastwindowtitle": "examples/simple-button.cpp · main · Sergey Bugaev / peel · GitLab — Mozilla Firefox",
    "ispersistent": false,
    "tiledLayout": "dwindle"
}])" };

  auto json{ nlohmann::json::parse(input) };

  for (const auto& [key, value] : json.items()) {
    const auto workspace{ value.get<Workspace>() };
    fmt::println("{}", workspace.last_window_title());
  }
}
