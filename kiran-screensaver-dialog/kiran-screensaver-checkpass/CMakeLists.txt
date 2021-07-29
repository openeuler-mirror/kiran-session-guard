cmake_minimum_required(VERSION 3.5)
project(kiran-screensaver-checkpass)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(PkgConfig REQUIRED)
pkg_search_module(KIRAN_LOG REQUIRED klog-qt5)

find_package(Qt5 COMPONENTS Core)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE SRC "*.cpp" "*.h")
file(GLOB_RECURSE PUBLIC_SRC "../checkpass-common/*.cpp" "public/*.h" )

add_executable(${PROJECT_NAME} ${SRC} ${PUBLIC_SRC})

target_include_directories(${PROJECT_NAME} PRIVATE
        ../checkpass-common)

target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        ${KIRAN_LOG_LIBRARIES}
        pam)

target_include_directories(${PROJECT_NAME} PRIVATE
        src
        ${KIRAN_LOG_INCLUDE_DIRS})