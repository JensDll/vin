add_library(
  vin.module
  STATIC
  workspace.cpp
  workspace_item.cpp
  time.cpp
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
    vin::deps::lua
    vin::deps::gtk_layer_shell
)

set(
  blueprints
  "${CMAKE_CURRENT_LIST_DIR}/workspace_item.blp"
  "${CMAKE_CURRENT_LIST_DIR}/workspace_list_item.blp"
)

set(
  uis
  "${gen_dir}/workspace_item.ui"
  "${gen_dir}/workspace_list_item.ui"
)

add_custom_command(
  OUTPUT
    ${uis}
  COMMAND
    "${BLUEPRINT_COMPILER_EXE}" batch-compile --minify "${gen_dir}" "${CMAKE_CURRENT_LIST_DIR}"
    ${blueprints}
  DEPENDS
    ${blueprints}
  COMMENT "Compiling blueprints"
  VERBATIM
  CODEGEN
)
add_custom_target(blueprints DEPENDS ${uis})

add_custom_command(
  OUTPUT
    "${gen_dir}/vin_resource.cpp"
  COMMAND
    "${glib_compile_resources}" "${CMAKE_CURRENT_LIST_DIR}/gresource.xml" --generate --target
    "${gen_dir}/vin_resource.cpp" --sourcedir "${gen_dir}" --internal
  DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/gresource.xml"
    ${uis}
    blueprints
  COMMENT "Compiling resources"
  VERBATIM
  CODEGEN
)
add_custom_target(resources DEPENDS "${gen_dir}/vin_resource.cpp")

add_dependencies(vin.module resources)
