cmake_minimum_required(VERSION 3.16)
set(PROJ_NAME "gsp")
project(${PROJ_NAME} VERSION 0.1 LANGUAGES CXX)
#C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # Allow following code in Neovim
# 1. Enable automation for Qt tools
set(CMAKE_AUTOUIC ON) # Automatically handles .ui files
set(CMAKE_AUTOMOC ON) # Automatically handles Meta-Object Compiler
set(CMAKE_AUTORCC ON) # Automatically handles .qrc (resources)
set(SS_ROOT  "/home/tim/Dev/C++/Astro/stellarsolver") # Where the Source Utils live
set(SS_UTILS   ${SS_ROOT}/ssolverutils) # Where the Source Utils live
set(SS_SEP     ${SS_ROOT}/stellarsolver/sep)
set(SS_ASTRO_ROOT ${SS_ROOT}/stellarsolver/astrometry)
set(SS_ASTRO_UTIL ${SS_ASTRO_ROOT}/util)
set(SS_ASTRO_HDR  ${SS_ASTRO_ROOT}/include)

find_package(Qt6 REQUIRED COMPONENTS Core Concurrent Gui Widgets Network)
message(STATUS "Using QT Include Path of ${Qt6Core_INCLUDE_DIRS}/")
# Find the CFITSIO package
find_package(cfitsio REQUIRED)
if (cfitsio_FOUND)
  message(STATUS "We have cfitsio")
else()
  message(Error "We do not have cfitsio")
endif()

find_package(PkgConfig REQUIRED)
find_package(StellarSolver)
pkg_check_modules(DC1394 REQUIRED libdc1394-2)
#pkg_check_modules(StellarSolver REQUIRED stellarsolver)  # Installed in /usr/local/lib/pkgconfig

if(${StellarSolver_FOUND})
  #I can not see these variables
  message(STATUS "StellarSolver lib     is ${StellarSolver_LIBRARIES}")
  message(STATUS "StellarSolver include is ${StellarSolver_INCLUDE_DIR}")
  set(StellarSolver_BUILD_TS 123)
  set(StellarSolver_VERSION_MAJOR 2)
  set(StellarSolver_VERSION_MINOR 7)
endif()

# Get all variable names into a list
get_cmake_property(_vars VARIABLES)
list(SORT _vars)

# Filter and print only variables starting with your package name
foreach(_v ${_vars})
  if(_v MATCHES "StellarSolver") # Replace MyPackage with your package name
    message(STATUS "${_v} = ${${_v}}")
  endif()
endforeach()

set(PROJECT_SOURCES main.cpp binning.cpp)
set(PROJECT_HEADER  nan.h binning.h)
message(STATUS "SS_ASTRO_ROOT is ${SS_ASTRO_ROOT}")
set(SS_ASTRO_SRC
  ${SS_ASTRO_UTIL}/bl.c
  ${SS_ASTRO_UTIL}/bl-nl.c
  ${SS_ASTRO_UTIL}/bl-nl-sort.c
  ${SS_ASTRO_UTIL}/bl-sort.c
  ${SS_ASTRO_UTIL}/codekd.c
  ${SS_ASTRO_UTIL}/datalog.c
  ${SS_ASTRO_UTIL}/errors.c
  ${SS_ASTRO_UTIL}/fitsbin.c
  ${SS_ASTRO_UTIL}/fitsfile.c
  ${SS_ASTRO_UTIL}/fitsioutils.c
  ${SS_ASTRO_UTIL}/fitstable.c
  ${SS_ASTRO_UTIL}/fit-wcs.c
  ${SS_ASTRO_UTIL}/gslutils.c
  ${SS_ASTRO_UTIL}/healpix.c
  ${SS_ASTRO_UTIL}/index.c
  ${SS_ASTRO_UTIL}/ioutils.c
  ${SS_ASTRO_UTIL}/log.c
  ${SS_ASTRO_UTIL}/mathutil.c
  ${SS_ASTRO_UTIL}/md5.c
  ${SS_ASTRO_UTIL}/os-features.c
  ${SS_ASTRO_UTIL}/os-features-test.c
  ${SS_ASTRO_UTIL}/permutedsort.c
  ${SS_ASTRO_UTIL}/qsort_reentrant.c
  ${SS_ASTRO_UTIL}/quadfile.c
  ${SS_ASTRO_UTIL}/sip.c
  ${SS_ASTRO_UTIL}/sip-utils.c
  ${SS_ASTRO_UTIL}/starkd.c
  ${SS_ASTRO_UTIL}/starutil.c
  ${SS_ASTRO_UTIL}/starxy.c
  ${SS_ASTRO_UTIL}/tic.c
  ${SS_ASTRO_UTIL}/md5.h)

set(SS_SEP_SRC
  ${SS_SEP}/analyse.cpp
  ${SS_SEP}/aperture.cpp
  ${SS_SEP}/background.cpp
  ${SS_SEP}/convolve.cpp
  ${SS_SEP}/deblend.cpp
  ${SS_SEP}/extract.cpp
  ${SS_SEP}/lutz.cpp
  ${SS_SEP}/util.cpp)

set(SS_SEP_HDR
  ${SS_SEP}/analyse.h
  ${SS_SEP}/deblend.h
  ${SS_SEP}/extract.h
  ${SS_SEP}/lutz.h
  ${SS_SEP}/overlap.h
  ${SS_SEP}/sepcore.h
  ${SS_SEP}/sep.h)

set(SS_UTIL_SRC ${SS_UTILS}/dms.cpp
  ${SS_UTILS}/fileio.cpp
  ${SS_UTILS}/imagelabel.cpp
  ${SS_UTILS}/stretch.cpp)

set(SS_UTIL_HDR ${SS_UTILS}/bayer.h
  ${SS_UTILS}/dms.h
  ${SS_UTILS}/fileio.h
  ${SS_UTILS}/imagelabel.h
  ${SS_UTILS}/stretch.h)

configure_file(
  ${SS_ROOT}/version.h.cmake
  "${CMAKE_CURRENT_BINARY_DIR}/version.h")

add_executable(${PROJ_NAME} ${PROJECT_SOURCES} ${PROJECT_HEADER}
                ${SS_UTIL_SRC} ${SS_UTIL_HDR}
                ${SS_SEP_SRC} ${SS_SEP_HDR}
                ${SS_ASTRO_SRC} ${SS_ASTRO_HDR})
message(STATUS "SS_ASTRO_HDR is ${SS_ASTRO_HDR}")

target_include_directories(${PROJ_NAME} PRIVATE Qt::Core
    Qt::Gui
    ${CMAKE_SOURCE_DIR}
    ${SS_ROOT}
    ${STELLASOLVER_INCLUDE_DIRS}
    ${SS_ROOT}/stellarsolver
    ${SS_ASTRO_HDR}
    ${CMAKE_BINARY_DIR})

target_link_libraries(${PROJ_NAME} PRIVATE
  Qt6::Concurrent
  Qt6::Gui
  Qt6::Widgets
  Qt6::Network
  cfitsio
  StellarSolver::stellarsolver
  ${DC1394_LIBRARIES})

include(GNUInstallDirs)
install(TARGETS ${PROJ_NAME}
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
