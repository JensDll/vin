cmake_minimum_required(VERSION 4.3)

project(VinDeps VERSION 0.1.0 LANGUAGES NONE)

set(project_root ${CMAKE_CURRENT_LIST_DIR}/..)
list(APPEND CMAKE_MODULE_PATH ${project_root}/cmake)

include(cpm)
include(ExternalProject)

option(VIN_BUILD_PEEL OFF "Build peel")
option(VIN_BUILD_FMT OFF "Build fmt")
option(VIN_BUILD_SPDLOG OFF "Build spdlog")
option(VIN_BUILD_GTK4_LAYER_SHELL OFF "Build gtk4 layer shell")
option(VIN_BUILD_JSON OFF "Build json")

set(
  meson_build_type
  $<IF:$<CONFIG:Release,MinSizeRel>,release,$<IF:$<CONFIG:RelWithDebInfo>,debugoptimized,debug>>
)

if(VIN_BUILD_PEEL OR VIN_BUILD_GTK4_LAYER_SHELL)
  find_program(MESON_EXE REQUIRED NAMES meson DOC "The Meson build system")
endif()

if(VIN_BUILD_PEEL)
  CPMAddPackage(
    NAME peel
    GIT_REPOSITORY https://gitlab.gnome.org/bugaevc/peel.git
    GIT_TAG 986a798be46acc1fed18ea7043143d4d9038c0ab
    GIT_SHALLOW ON
    DOWNLOAD_ONLY ON
  )

  ExternalProject_Add(
    peel
    SOURCE_DIR ${peel_SOURCE_DIR}
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    CONFIGURE_COMMAND
      ${MESON_EXE} setup <BINARY_DIR> <SOURCE_DIR> --prefix <INSTALL_DIR> --buildtype
      ${meson_build_type}
    BUILD_COMMAND
      ${MESON_EXE} compile -C <BINARY_DIR>
    INSTALL_COMMAND
      ""
  )

  ExternalProject_Get_Property(
    peel
    BINARY_DIR
  )

  set(install_peel "COMMAND ${MESON_EXE} install -C ${BINARY_DIR}")
endif()

if(VIN_BUILD_GTK4_LAYER_SHELL)
  CPMAddPackage(
    NAME gtk4_layer_shell
    GIT_REPOSITORY https://github.com/wmww/gtk4-layer-shell.git
    GIT_TAG 2a70cb0347e558f9d4e004a78e11e34cd680fa24
    GIT_SHALLOW ON
    DOWNLOAD_ONLY ON
  )

  ExternalProject_Add(
    gtk4_layer_shell
    SOURCE_DIR ${gtk4_layer_shell_SOURCE_DIR}
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    CONFIGURE_COMMAND
      ${MESON_EXE} setup <BINARY_DIR> <SOURCE_DIR> --prefix <INSTALL_DIR> --buildtype
      ${meson_build_type} -D vapi=false
    BUILD_COMMAND
      ${MESON_EXE} compile -C <BINARY_DIR>
    INSTALL_COMMAND
      ""
  )

  ExternalProject_Get_Property(
    gtk4_layer_shell
    BINARY_DIR
  )

  set(install_gtk4_layer_shell "COMMAND ${MESON_EXE} install -C ${BINARY_DIR}")
endif()

set(spdlog_options "SPDLOG_INSTALL ON")

if(VIN_BUILD_FMT)
  CPMAddPackage(
    NAME fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 407c905e45ad75fc29bf0f9bb7c5c2fd3475976f
    GIT_SHALLOW ON
    OPTIONS
      "FMT_INSTALL ON"
  )
  list(APPEND spdlog_options "SPDLOG_FMT_EXTERNAL ON")
endif()

if(VIN_BUILD_SPDLOG)
  CPMAddPackage(
    NAME spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG 79524ddd08a4ec981b7fea76afd08ee05f83755d
    GIT_SHALLOW ON
    OPTIONS
      ${spdlog_options}
  )
endif()

if(VIN_BUILD_JSON)
  CPMAddPackage(
    NAME json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG a0a4e7cc0b2bab727b2b6e7505a52b0e7e38f063
    GIT_SHALLOW ON
    OPTIONS
      "JSON_Install ON"
  )
endif()

configure_file(install.cmake.in install.cmake @ONLY)

install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/install.cmake")
