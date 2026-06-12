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
option(VIN_BUILD_GLIB OFF "Build GLib")
option(VIN_BUILD_GTK OFF "Build GTK")

set(
  meson_build_type
  $<IF:$<CONFIG:Release,MinSizeRel>,release,$<IF:$<CONFIG:RelWithDebInfo>,debugoptimized,debug>>
)

if(VIN_BUILD_PEEL OR VIN_BUILD_GTK4_LAYER_SHELL OR VIN_BUILD_GTK)
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
    URL
      https://github.com/fmtlib/fmt/archive/refs/tags/12.1.0.tar.gz
    URL_HASH
      SHA512=f0da82c545b01692e9fd30fdfb613dbb8dd9716983dcd0ff19ac2a8d36f74beb5540ef38072fdecc1e34191b3682a8542ecbf3a61ef287dbba0a2679d4e023f2
    OPTIONS
      "FMT_INSTALL ON"
  )
  list(APPEND spdlog_options "SPDLOG_FMT_EXTERNAL ON")
endif()

if(VIN_BUILD_SPDLOG)
  CPMAddPackage(
    NAME spdlog
    URL
      https://github.com/gabime/spdlog/archive/refs/tags/v1.17.0.tar.gz
    URL_HASH
      SHA512=8df117055d19ff21c9c9951881c7bdf27cc0866ea3a4aa0614b2c3939cedceab94ac9abaa63dc4312b51562b27d708cb2f014c68c603fd1c1051d3ed5c1c3087
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

if(VIN_BUILD_GLIB)
  CPMAddPackage(
    NAME glib
    URL
      https://gitlab.gnome.org/GNOME/glib/-/archive/2.89.0/glib-2.89.0.tar.gz
    URL_HASH
      SHA512=22448250034297742c8e6e9a16ccfefa621786257cc141e6e2fc42bf6ef5ea6cc0968b8a49b5f698eca785dd1390d6252cce50758a0c67c128c3151b4a6aef4f
    DOWNLOAD_ONLY ON
  )
endif()

if(VIN_BUILD_GTK)
  CPMAddPackage(
    NAME gtk
    URL
      https://gitlab.gnome.org/GNOME/gtk/-/archive/4.23.1/gtk-4.23.1.tar.gz
    URL_HASH
      SHA512=8a47e020ee0c06f1adbf66ddb9f6f1de9c33485787896951e71d5a4d12810ad471d8ad379697dfe71397ba23015114751f29fafc37f9e6f7c4e184a9077bc96d
    DOWNLOAD_ONLY ON
  )

  ExternalProject_Add(
    gtk
    SOURCE_DIR "${gtk_SOURCE_DIR}"
    INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
    CONFIGURE_COMMAND
      ${MESON_EXE} setup <BINARY_DIR> <SOURCE_DIR> --prefix <INSTALL_DIR> --buildtype
      ${meson_build_type} -D build-demos=false -D build-testsuite=false -D build-examples=false -D
      build-tests=false -D introspection=enabled
    BUILD_COMMAND
      ${MESON_EXE} compile -C <BINARY_DIR>
    INSTALL_COMMAND
      ""
  )

  ExternalProject_Get_Property(
    gtk
    BINARY_DIR
  )

  set(install_gtk "${MESON_EXE} install -C ${BINARY_DIR}")
endif()

configure_file(install.cmake.in install.cmake @ONLY)

install(SCRIPT "${CMAKE_CURRENT_BINARY_DIR}/install.cmake")
