include_guard()

list(APPEND CMAKE_MESSAGE_CONTEXT deps)

find_package(PkgConfig REQUIRED)
find_package(Python3 REQUIRED)
find_package(peel CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

if(VIN_BUILD_TESTING)
  find_package(Catch2 CONFIG REQUIRED)
  add_library(vin::deps::catch2 ALIAS Catch2::Catch2)
  add_library(vin::deps::catch2_with_main ALIAS Catch2::Catch2WithMain)
  include(Catch)
endif()

pkg_check_modules(gio2 REQUIRED IMPORTED_TARGET gio-2.0)

pkg_get_variable(glib_compile_schemas gio-2.0 glib_compile_schemas)
pkg_get_variable(glib_compile_resources gio-2.0 glib_compile_resources)

if(NOT glib_compile_schemas)
  message(FATAL_ERROR "Failed to find program glib-compile-schemas")
endif()

if(NOT glib_compile_resources)
  message(FATAL_ERROR "Failed to find program glib-compile-resources")
endif()

pkg_check_modules(gtk4 REQUIRED IMPORTED_TARGET gtk4)

pkg_check_modules(gtk4_layer_shell REQUIRED IMPORTED_TARGET gtk4-layer-shell-0)

pkg_search_module(
  lua
  REQUIRED
  IMPORTED_TARGET
  lua55
  lua5.5
  lua-55
  lua-5.5
  lua>=5.5
  lua<5.6
)

find_program(
  BLUEPRINT_COMPILER_EXE
  REQUIRED
  NAMES
    blueprint-compiler
  DOC "A markup language compiler for GTK user interfaces"
)

find_program(PEEL_GEN_EXE REQUIRED NAMES peel-gen DOC "Program to generate GTK bindings")

set(gen_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(gen_dir_include "${gen_dir}/include")

file(MAKE_DIRECTORY "${gen_dir_include}")

add_custom_command(
  OUTPUT
    "${gen_dir_include}/peel"
  COMMAND
    "${CMAKE_COMMAND}" -E env
    "$<$<BOOL:${CMAKE_PREFIX_PATH}>:GI_GIR_PATH=${CMAKE_PREFIX_PATH}/share/gir-1.0>" --
    "${Python3_EXECUTABLE}" "${PEEL_GEN_EXE}" --recursive Gtk 4.0 Gtk4LayerShell 1.0
  WORKING_DIRECTORY "${gen_dir_include}"
  VERBATIM
  CODEGEN
  COMMENT "Running peel-gen"
)
add_custom_target(peel_gen DEPENDS "${gen_dir_include}/peel")

add_library(vin.deps.peel INTERFACE IMPORTED)
target_include_directories(vin.deps.peel INTERFACE "${gen_dir_include}")
target_link_libraries(vin.deps.peel INTERFACE peel::peel)
add_dependencies(vin.deps.peel peel_gen)

add_library(vin::deps::peel ALIAS vin.deps.peel)
add_library(vin::deps::json ALIAS nlohmann_json::nlohmann_json)
add_library(vin::deps::fmt ALIAS fmt::fmt)
add_library(vin::deps::spdlog ALIAS spdlog::spdlog)
add_library(vin::deps::gtk ALIAS PkgConfig::gtk4)
add_library(vin::deps::gio ALIAS PkgConfig::gio2)
add_library(vin::deps::gtk_layer_shell ALIAS PkgConfig::gtk4_layer_shell)
add_library(vin::deps::lua ALIAS PkgConfig::lua)

list(POP_BACK CMAKE_MESSAGE_CONTEXT)
