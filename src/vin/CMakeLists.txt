add_subdirectory(hyprland)

add_executable(
  vin
  main.cpp
  vin.cpp
)
target_link_libraries(
  vin
  PRIVATE
    vin::options
    vin::hyprland
    vin::deps::json
    vin::deps::spdlog
    vin::deps::peel
    vin::deps::gtk4
    vin::deps::gtk4_layer_shell
)

add_executable(test_main test_main.cpp)
target_link_libraries(
  test_main
  PRIVATE
    vin::options
    vin::deps::json
    vin::deps::spdlog
)
