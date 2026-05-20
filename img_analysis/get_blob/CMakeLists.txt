cmake_minimum_required(VERSION 4.1)
# Set up the project
project(indiscript)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# Set the C++ standard to 20
set(CMAKE_CXX_STANDARD 20)

# Ensure that the compiler MUST support C++20 to build
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optional: Disable compiler-specific extensions (e.g., gnu++20)
set(CMAKE_CXX_EXTENSIONS OFF)

# INDI and OpenCV flags
# Find pkg-config system tool
# We need this tooling
find_package(Qt6 REQUIRED COMPONENTS Core Gui)
find_package(PkgConfig REQUIRED)
#Now using PkgConfig find libindi
pkg_check_modules(INDI REQUIRED libindi)
message(STATUS "Using QT Include Path of ${Qt6Core_INCLUDE_DIRS} ")
find_library(QT_INDI_CLI_LIB  NAMES indiclientqt PATHS /usr/local/lib)
set(SRC main.cpp DeviceController.cpp indi_ccd_cli.cpp )
set(HDR DeviceController.h indi_ccd_cli.h)
add_executable(indiblob ${SRC}
                        ${HDR}        # optional, just for IDEs
                          )
# Enable automatic MOC, UIC, RCC
set_target_properties(indiblob PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

target_include_directories(indiblob PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}  # your headers live here
    ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Gui_INCLUDE_DIRS}
    ${INDI_INCLUDE_DIRS}         # INDI includes
    /usr/local/include/libindi
)

target_link_libraries(indiblob PRIVATE
    ${INDI_LIBRARIES}            # INDI libraries
    ${Qt6Core_LIBRARIES}
)

