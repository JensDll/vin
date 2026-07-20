set(gresource_cpp "${gen_dir}/gresource.cpp")
set(gresource_xml "${CMAKE_CURRENT_LIST_DIR}/gresource.xml")

add_executable(
  vin
  config.cpp
  main.cpp
  application.cpp
  ipositionable.cpp
  iconfigurable.cpp
  window.cpp
  hyprland_command.cpp
  hyprland_event_listener.cpp
  hyprland_workspace.cpp
  hyprland_workspace_item.cpp
  hyprland_workspace_list_model.cpp
  hyprland_workspace_module.cpp
  time_module.cpp
  notification_server.cpp
  notification.cpp
  notification_item.cpp
  notification_list_model.cpp
  notification_module.cpp
  notification_window.cpp
  "${gresource_cpp}"
)

target_link_libraries(
  vin
  PRIVATE
    vin::options
    vin::deps::gtk_layer_shell
    vin::deps::lua
    vin::deps::spdlog
    vin::deps::peel
    vin::deps::gtk
)

if(VIN_BUILD_TESTING)
  add_executable(
    vin.test
    notification.cpp
    notification.test.cpp
  )
  target_link_libraries(
    vin.test
    PUBLIC
      vin::options
      vin::deps::spdlog
      vin::deps::peel
      vin::deps::gtk
      vin::deps::catch2_with_main
  )
  target_compile_definitions(vin.test PRIVATE VIN_NOTIFICATION_TAG="[vin][notification]")
  catch_discover_tests(vin.test)
endif()

set(
  blueprints
  "${CMAKE_CURRENT_LIST_DIR}/window.blp"
  "${CMAKE_CURRENT_LIST_DIR}/hyprland_workspace_module.blp"
  "${CMAKE_CURRENT_LIST_DIR}/hyprland_workspace_item.blp"
  "${CMAKE_CURRENT_LIST_DIR}/time_module.blp"
  "${CMAKE_CURRENT_LIST_DIR}/notification_window.blp"
  "${CMAKE_CURRENT_LIST_DIR}/notification_item.blp"
)

set(
  uis
  "${gen_dir}/window.ui"
  "${gen_dir}/hyprland_workspace_module.ui"
  "${gen_dir}/hyprland_workspace_item.ui"
  "${gen_dir}/time_module.ui"
  "${gen_dir}/notification_window.ui"
  "${gen_dir}/notification_item.ui"
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
add_custom_target(vin.blueprint.gen DEPENDS ${uis})

add_custom_command(
  OUTPUT
    "${gresource_cpp}"
  COMMAND
    "${glib_compile_resources}" "${gresource_xml}" --generate --target "${gresource_cpp}"
    --sourcedir "${gen_dir}" --internal
  DEPENDS
    "${gresource_xml}"
    ${uis}
    vin.blueprint.gen
  COMMENT "Compiling resources"
  VERBATIM
  CODEGEN
)
add_custom_target(vin.resource.gen DEPENDS "${gresource_cpp}")

set_source_files_properties(
  "${gresource_cpp}"
  PROPERTIES
    SKIP_LINTING
      ON
)

install(TARGETS vin)
