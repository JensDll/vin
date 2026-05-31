add_library(
  vin.module
  STATIC
  time_module.cpp
  workspace_module.cpp
)
add_library(vin::module ALIAS vin.module)
target_link_libraries(
  vin.module
  PRIVATE
    vin::options
    vin::lib
    vin::deps::spdlog
    vin::deps::peel
    vin::deps::gtk
)
