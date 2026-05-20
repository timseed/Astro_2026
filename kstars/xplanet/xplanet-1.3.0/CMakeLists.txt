cmake_minimum_required(VERSION 4.0)
project(xplanet_cmake_bld VERSION 1.0.1)
# Output config files so I can followingin neovim
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FT REQUIRED freetype2)
pkg_check_modules(PANGO REQUIRED pango)

set( libdisp_src ${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/DisplayX11.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/Timer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/TimerX11.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/TextRendererFT2.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/DisplayOutput.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/getTimer.cpp
#${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/TimerMacAqua.cpp
#${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/DisplayMacAqua.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/DisplayBase.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/getDisplay.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/TextRenderer.cpp
#${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/DisplayMSWin.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/getTextRenderer.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/TextRendererPangoFT2.cpp )

set (libdisp_hdr  ${CMAKE_CURRENT_SOURCE_DIR}/config.h
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/TimerMacAqua.h
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/DisplayX11.h
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/DisplayBase.h
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/libdisplay.h
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/libtimer.h
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/TextRendererPangoFT2.h
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/TimerX11.h
#${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/DisplayMSWin.h
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/vroot.h
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/TextRendererFT2.h
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/DisplayOutput.h
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/Timer.h
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/DisplayMacAqua.h
${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay/TextRenderer.h)

add_library(libdisplay SHARED ${libdisp_src})
set_target_properties(libdisplay PROPERTIES VERSION ${PROJECT_VERSION})
target_include_directories(libdisplay PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/libdisplay
  ${PANGO_INCLUDE_DIRS}
  ${FT_INCLUDE_DIRS})
