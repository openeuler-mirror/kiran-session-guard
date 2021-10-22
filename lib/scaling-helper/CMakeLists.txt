set(TARGET_NAME scaling-helper)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core)
pkg_search_module(KIRANLOG REQUIERD klog-qt5)
pkg_search_module(XTST REQUIRED xtst)
pkg_search_module(X11 REQUIRED x11)
pkg_search_module(XRANDR REQUIRED xrandr)

file(GLOB_RECURSE ALL_FILES ./*.h ./*.cpp)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_library(${TARGET_NAME} STATIC
        ${ALL_FILES})

target_link_libraries(${TARGET_NAME} PUBLIC
        Qt5::Core
        ${KIRANLOG_LIBRARIES}
        ${XTST_LIBRARIES}
        ${X11_LIBRARIES}
        ${XRANDR_LIBRARIES})

target_include_directories(${TARGET_NAME} PRIVATE
        ${KIRANLOG_INCLUDE_DIRS}
        ${XTST_INCLUDE_DIRS}
        ${X11_INCLUDE_DIRS}
        ${XRANDR_INCLUDE_DIRS})

target_include_directories(${TARGET_NAME} INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR})