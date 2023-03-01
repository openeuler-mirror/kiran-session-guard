set (TARGET_NAME common-widgets)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets)
pkg_search_module(KIRANLOG REQUIERD klog-qt5)
pkg_search_module(KIRAN_STYLE_HELPER REQUIRED kiran-style-helper)

# Qt编译设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE COMMON_WIDGETS_SRC ./*.h ./*.cpp)
file(GLOB_RECURSE COMMON_WIDGETS_QRC resources/commonWidgets.qrc)

add_library(${TARGET_NAME} STATIC ${COMMON_WIDGETS_SRC} ${COMMON_WIDGETS_QRC})

target_include_directories(${TARGET_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${KIRANLOG_INCLUDE_DIRS}
        ${KIRAN_STYLE_HELPER_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(${TARGET_NAME} INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${TARGET_NAME} PUBLIC
        Qt5::Core
        Qt5::Widgets
        ${KIRAN_STYLE_HELPER_LIBRARIES}
        ${KIRANLOG_LIBRARIES})
