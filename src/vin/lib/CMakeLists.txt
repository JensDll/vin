add_library(
  vin.lib
  STATIC
  hyprland_command.cpp
  hyprland_event_listener.cpp
  hyprland_workspace.cpp
  hyprland_workspace_list_model.cpp
  ipositionable.cpp
  iconfigurable.cpp
  notification_server.cpp
  notification.cpp
  notification_list_model.cpp
)
add_library(vin::lib ALIAS vin.lib)
target_link_libraries(
  vin.lib
  PRIVATE
    vin::options
    vin::deps::spdlog
    vin::deps::peel
    vin::deps::gtk
)

if(VIN_BUILD_TESTING)
  add_executable(vin.lib.test notification.test.cpp)
  target_link_libraries(
    vin.lib.test
    PUBLIC
      vin.lib
      vin::options
      vin::deps::fmt
      vin::deps::peel
      vin::deps::gtk
      vin::deps::catch2_with_main
  )
  target_compile_definitions(vin.lib.test PRIVATE VIN_TEST_TAG="[vin][lib]")
  catch_discover_tests(vin.lib.test)
endif()
