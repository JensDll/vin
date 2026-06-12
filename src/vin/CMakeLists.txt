add_subdirectory(lib)
add_subdirectory(module)

add_executable(
  vin
  config.cpp
  main.cpp
  application.cpp
  window.cpp
  "${gen_dir}/vin_resource.cpp"
)
target_link_libraries(
  vin
  PRIVATE
    vin::options
    vin::lib
    vin::module
    vin::deps::gtk_layer_shell
    vin::deps::lua
    vin::deps::spdlog
    vin::deps::peel
    vin::deps::gtk
)

set_source_files_properties(
  "${gen_dir}/vin_resource.cpp"
  PROPERTIES
    SKIP_LINTING
      ON
)

install(TARGETS vin)
