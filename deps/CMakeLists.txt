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
option(VIN_BUILD_GLIB OFF "Build json")
option(VIN_BUILD_CATCH2 OFF "Build Catch2")

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
    GIT_TAG f3b3b300e0f83013553b54125e23b8e53f555dba
    GIT_SHALLOW ON
    DOWNLOAD_ONLY ON
  )

  ExternalProject_Add(
    peel
    SOURCE_DIR "${peel_SOURCE_DIR}"
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
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

  set(install_peel "${MESON_EXE} install -C ${BINARY_DIR}")
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
    SOURCE_DIR "${gtk4_layer_shell_SOURCE_DIR}"
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
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

  set(install_gtk4_layer_shell "${MESON_EXE} install -C ${BINARY_DIR}")
endif()

set(spdlog_options "SPDLOG_INSTALL ON")

if(VIN_BUILD_FMT)
  CPMAddPackage(
    NAME fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 2f18a88e68af42bdda0e5210f80fac07ae756bd6
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
    GIT_TAG 1d7688aef2e7b6f9d11644702f8ce7533aa1fb20
    OPTIONS
      "JSON_Install ON"
  )
endif()

if(VIN_BUILD_CATCH2)
  CPMAddPackage(
    NAME catch2
    URL
      https://github.com/catchorg/Catch2/archive/refs/tags/v3.15.0.tar.gz
    URL_HASH
      SHA512=09df8b291d0c756c492871ce0a4fb5c703162d1364a9864470b227d4d19d9865a0ba5293a5f38d0117cbc7c230f42be35db9f6e0ca5d889dff02f1745807f380
    DOWNLOAD_ONLY ON
  )

  # Use external project because it's not installable when added via add_subdirectory
  ExternalProject_Add(
    catch2
    SOURCE_DIR "${catch2_SOURCE_DIR}"
    CONFIGURE_COMMAND
      ${CMAKE_COMMAND} -B <BINARY_DIR> -S <SOURCE_DIR> --install-prefix <INSTALL_DIR> -D
      CMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE} -D CATCH_INSTALL_DOCS:BOOL=OFF
    BUILD_COMMAND
      ${CMAKE_COMMAND} --build <BINARY_DIR>
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
    INSTALL_COMMAND
      ""
  )

  ExternalProject_Get_Property(
    catch2
    BINARY_DIR
  )

  set(install_catch2 "${CMAKE_COMMAND} --build ${BINARY_DIR} --target install")
endif()

configure_file(install.cmake.in install.cmake @ONLY)

install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/install.cmake")
