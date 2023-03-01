set (TARGET_NAME login-frame)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets)
pkg_search_module(KIRANLOG REQUIERD klog-qt5)

# Qt编译设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE LOGIN_FRAME_SRC ./*.h ./*.cpp)
file(GLOB_RECURSE LOGIN_FRAME_QRC resources/loginFrame.qrc)

add_library(${TARGET_NAME} STATIC
        ${LOGIN_FRAME_SRC}
        ${LOGIN_FRAME_QRC})

target_include_directories(${TARGET_NAME} PRIVATE
        ${KIRANLOG_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(${TARGET_NAME} INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${TARGET_NAME} PUBLIC
        auth-proxy
        common-widgets
        utils
        dbus
        Qt5::Core
        Qt5::Widgets
        ${KIRANLOG_LIBRARIES})

