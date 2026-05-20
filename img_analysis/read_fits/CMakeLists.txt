cmake_minimum_required(VERSION 3.16)
project(IndiQtClient)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-as-needed")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 REQUIRED COMPONENTS Core Concurrent Network Widgets)
# We need this tooling
find_package(PkgConfig REQUIRED)
#Note Nicer IMPORTED_TARGET directive
pkg_check_modules(FITS REQUIRED IMPORTED_TARGET cfitsio)


set(SRC main.cpp count_stars.cpp MyFits.cpp)
set(HDR count_stars.h MyFits.h)

add_executable(read_fits ${SRC} ${HDR})

#target_include_directories(read_fits PRIVATE ${INDI_INCLUDE_DIRS})

target_link_libraries(read_fits
    PkgConfig::FITS
    Qt6::Core
    Qt6::Concurrent
)

add_custom_target(run
    COMMAND read_fits test.fits 
    DEPENDS read_fits
)
