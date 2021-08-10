set (TARGET_NAME auth-proxy)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core DBus)
pkg_search_module(KIRANLOG REQUIERD klog-qt5)
pkg_search_module(LIGHTDM_QT5 REQUIRED liblightdm-qt5-3)

# Qt编译设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#生成使用的DBus接口源文件
set(DBUS_INTERFACE_SRC "")
qt5_add_dbus_interface(DBUS_INTERFACE_SRC data/com.kylinsec.Kiran.SystemDaemon.Authentication.xml kiran_authentication)
foreach(interface_item ${DBUS_INTERFACE_SRC})
    set_property(SOURCE ${interface_item} PROPERTY SKIP_AUTOGEN ON)
endforeach()

file(GLOB_RECURSE ALL_FILES ./*.h ./*.cpp)
file(GLOB_RECURSE CHECK_PASS_COMMON_SRC ../../checkpass-common/*.cpp ../../checkpass-common/*.h)

include_directories(../../checkpass-common ${CMAKE_CURRENT_BINARY_DIR})

add_library(${TARGET_NAME}
        ${ALL_FILES}
        ${CHECK_PASS_COMMON_SRC}
        ${DBUS_INTERFACE_SRC})

target_include_directories(${TARGET_NAME} PUBLIC
        ${KIRANLOG_INCLUDE_DIRS}
        ${LIGHTDM_QT5_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME} PUBLIC
        Qt5::Core
        Qt5::DBus
        ${KIRANLOG_LIBRARIES}
        ${LIGHTDM_QT5_LIBRARIES})

