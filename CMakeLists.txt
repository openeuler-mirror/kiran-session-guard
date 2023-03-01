cmake_minimum_required(VERSION 3.5)

project(kiran-session-guard)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(GNUInstallDirs)
include(qdbus-wrapper)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(./lib)
add_subdirectory(./src)
# add_subdirectory(./lightdm-greeter)
# add_subdirectory(./screensaver-dialog)
# add_subdirectory(./polkit-agent)

