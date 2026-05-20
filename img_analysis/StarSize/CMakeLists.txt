cmake_minimum_required(VERSION 4.1)

project(StarSize VERSION 0.1 LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) #This helps neovim find the includes and lets the editor be more context sensative
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
# Enable automatic resource compilation
set(CMAKE_AUTORCC ON)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# We need this tooling
find_package(Qt6 REQUIRED COMPONENTS Core)
set(PROJECT_SRC
        main.cpp
        qtsextractor.cpp
        resource_copy.cpp
)
set(PROJECT_HDR
        qtsextractor.h
        resource_copy.h
)
set(PROJECT_RESOURCE
    resource.qrc
)

add_executable(StarSize ${PROJECT_SRC} ${PROJECT_HDR} ${PROJECT_RESOURCE})

target_include_directories(StarSize PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}  # your headers live here
    ${Qt6Core_INCLUDE_DIRS}
)


target_link_libraries(StarSize PRIVATE
    ${Qt6Core_LIBRARIES}
)
