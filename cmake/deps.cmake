include_guard()

list(APPEND CMAKE_MESSAGE_CONTEXT deps)

find_package(PkgConfig REQUIRED)
find_package(Python3 REQUIRED)
find_package(peel CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

pkg_check_modules(gtk4 REQUIRED IMPORTED_TARGET gtk4)

pkg_check_modules(
  wayland
  REQUIRED
  IMPORTED_TARGET
  wayland-client
  wayland-server
)

pkg_check_modules(gtk4_layer_shell REQUIRED IMPORTED_TARGET gtk4-layer-shell-0)

pkg_get_variable(wayland_protocols_data_dir wayland-protocols pkgdatadir)

find_program(PEEL_GEN_EXE REQUIRED NAMES peel-gen DOC "Program to generate GTK bindings")

find_program(WAYLAND_SCANNER_EXE REQUIRED NAMES wayland-scanner)

find_file(
  XDG_SHELL_XML
  REQUIRED
  NAMES
    xdg-shell.xml
  HINTS
    "${wayland_protocols_data_dir}/stable/xdg-shell"
)

set(gen_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(gen_dir_src "${gen_dir}/src")
set(gen_dir_include "${gen_dir}/include")

file(
  MAKE_DIRECTORY
    ${gen_dir_src}
    ${gen_dir_include}
)

set(xdg_shell_private_code "${gen_dir_src}/xdg-shell-protocol.c")
set(xdg_shell_client_header "${gen_dir_include}/xdg-shell-client-protocol.h")
add_custom_command(
  OUTPUT
    ${xdg_shell_private_code}
    ${xdg_shell_client_header}
  COMMAND
    ${WAYLAND_SCANNER_EXE} private-code ${XDG_SHELL_XML} ${xdg_shell_private_code}
  COMMAND
    ${WAYLAND_SCANNER_EXE} client-header ${XDG_SHELL_XML} ${xdg_shell_client_header}
  VERBATIM
  CODEGEN
  COMMENT "Running wayland-scanner"
)
add_custom_target(
  wayland_scanner
  DEPENDS
    ${xdg_shell_private_code}
    ${xdg_shell_client_header}
)

add_custom_command(
  OUTPUT
    "${gen_dir_include}/peel"
  COMMAND
    ${Python3_EXECUTABLE} ${PEEL_GEN_EXE} --recursive Gtk 4.0 GdkWayland 4.0 GLib 2.0 Gtk4LayerShell
    1.0
  WORKING_DIRECTORY ${gen_dir_include}
  VERBATIM
  CODEGEN
  COMMENT "Running peel-gen"
)
add_custom_target(peel_gen DEPENDS "${gen_dir_include}/peel")

add_library(vin.deps.wayland.xdg_shell INTERFACE IMPORTED)
target_include_directories(vin.deps.wayland.xdg_shell INTERFACE ${gen_dir_include})
target_sources(vin.deps.wayland.xdg_shell INTERFACE ${xdg_shell_private_code})
add_dependencies(vin.deps.wayland.xdg_shell wayland_scanner)

add_library(vin.deps.peel INTERFACE IMPORTED)
target_include_directories(vin.deps.peel INTERFACE ${gen_dir_include})
target_link_libraries(vin.deps.peel INTERFACE peel::peel)
add_dependencies(vin.deps.peel peel_gen)

add_library(vin::deps::wayland ALIAS PkgConfig::wayland)
add_library(vin::deps::wayland::xdg_shell ALIAS vin.deps.wayland.xdg_shell)

add_library(vin::deps::peel ALIAS vin.deps.peel)

add_library(vin::deps::json ALIAS nlohmann_json::nlohmann_json)

add_library(vin::deps::fmt ALIAS fmt::fmt)

add_library(vin::deps::spdlog ALIAS spdlog::spdlog)

add_library(vin::deps::gtk4 ALIAS PkgConfig::gtk4)

add_library(vin::deps::gtk4_layer_shell ALIAS PkgConfig::gtk4_layer_shell)

list(POP_BACK CMAKE_MESSAGE_CONTEXT)
