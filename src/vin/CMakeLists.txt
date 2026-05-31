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
