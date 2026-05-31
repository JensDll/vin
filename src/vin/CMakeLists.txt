add_subdirectory(lib)
add_subdirectory(module)

add_executable(
  vin
  config.cpp
  main.cpp
  application.cpp
  window.cpp
)

target_link_libraries(
  vin
  PRIVATE
    vin::options
    vin::module
    vin::lib
    vin::deps::lua
    vin::deps::spdlog
    vin::deps::peel
    vin::deps::gtk
    vin::deps::gtk_layer_shell
)

add_executable(test_main test_main.cpp)

target_link_libraries(
  test_main
  PRIVATE
    vin::options
    vin::deps::fmt
)
