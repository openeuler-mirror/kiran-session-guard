set (TARGET_NAME common)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets)
pkg_search_module(KIRANLOG REQUIERD klog-qt5)

# Qt编译设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE COMMON_SRC ./*.h ./*.cpp)
add_library(${TARGET_NAME} STATIC ${COMMON_SRC})

target_include_directories(${TARGET_NAME} PRIVATE
        ${KIRANLOG_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(${TARGET_NAME} INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${TARGET_NAME} PUBLIC
        Qt5::Core
        Qt5::Widgets
        ${KIRANLOG_LIBRARIES})

