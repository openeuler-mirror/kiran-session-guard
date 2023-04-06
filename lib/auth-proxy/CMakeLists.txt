set (TARGET_NAME auth-proxy)

set(DBUS_INTERFACE_SRC "")
kiran_qt5_add_dbus_interface_ex(DBUS_INTERFACE_SRC 
        data/com.kylinsec.Kiran.SystemDaemon.Authentication.xml
        kiran_authentication
        KiranAuthentication)

file(GLOB_RECURSE ALL_FILES ./*.h ./*.cpp)
file(GLOB_RECURSE CHECK_PASS_COMMON_SRC ../../checkpass-common/*.cpp ../../checkpass-common/*.h)

add_library(${TARGET_NAME} STATIC
        ${ALL_FILES}
        ${CHECK_PASS_COMMON_SRC}
        ${DBUS_INTERFACE_SRC})

target_include_directories(${TARGET_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}
        ${KLOG_QT5_INCLUDE_DIRS})

target_include_directories(${TARGET_NAME} INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${TARGET_NAME} PUBLIC
        Qt5::Core
        Qt5::DBus
        ${KLOG_QT5_LIBRARIES})

