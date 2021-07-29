cmake_minimum_required(VERSION 3.5)

project(kiran-screensaver-dialog)

option(HAVE_BIOMETRICS_AUTH "have biometrics auth" OFF)
option(SHOW_VIRTUAL_KEYBOARD "show virtual keyboard" OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(GNUInstallDirs)
set(SCREENSAVER_DIALOG_INSTALL ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBEXECDIR}/)
set(SCREENSAVER_QM_INSTALL ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/translations/)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Widgets X11Extras DBus Network LinguistTools)

pkg_search_module(GIO_2 REQUIRED gio-2.0)
pkg_search_module(GLIB_2 REQUIRED glib-2.0)
pkg_search_module(X11 REQUIRED x11)
pkg_search_module(XCB REQUIRED xcb)
pkg_search_module(XRANDR REQUIRED xrandr)
pkg_search_module(ZLOG REQUIRED zlog)
pkg_search_module(KIRAN_LOG REQUIRED klog-qt5)
pkg_search_module(KIRAN_WIDGETS REQUIRED kiranwidgets-qt5)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#生成使用的DBus接口源文件
set(DBUS_INTERFACE_SRC "")
qt5_add_dbus_interface(DBUS_INTERFACE_SRC data/com.kylinsec.Kiran.SystemDaemon.Authentication.xml kiran_authentication)
foreach(interface_item ${DBUS_INTERFACE_SRC})
    set_property(SOURCE ${interface_item} PROPERTY SKIP_AUTOGEN ON)
endforeach()

file(GLOB_RECURSE SRC "src/*.cpp" "src/*.h" "src/*.ui")
file(GLOB_RECURSE PUBLIC_SRC "public/*.cpp" "public/*.h")
# 资源文件
file(GLOB_RECURSE RESOURCES "resources/*.qrc")

set( TS_FILES "translations/kiran-screensaver-dialog.zh_CN.ts" )
qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES} )

if(HAVE_BIOMETRICS_AUTH)
    add_definitions(-DBIOMETRICS_AUTH)
endif()

if(SHOW_VIRTUAL_KEYBOARD)
    add_definitions(-DVIRTUAL_KEYBOARD)
endif()

include_directories("${CMAKE_BINARY_DIR}")

add_executable(${PROJECT_NAME} ${SRC} ${PUBLIC_SRC} ${RESOURCES} ${DBUS_INTERFACE_SRC} ${QM_FILES})

target_link_libraries(${PROJECT_NAME}
        Qt5::Widgets
        Qt5::X11Extras
        Qt5::DBus
        Qt5::Network
        ${GIO_2_LIBRARIES}
        ${GLIB_2_LIBRARIES}
        ${X11_LIBRARIES}
        ${XRANDR_LIBRARIES}
        ${ZLOG_LIBRARIES}
        ${KIRAN_LOG_LIBRARIES}
        ${KIRAN_WIDGETS_LIBRARIES}
        pam)

target_include_directories(${PROJECT_NAME} PRIVATE
        "src"
        "src/dbus-api-wrapper"
        "src/widgets"
        "src/tool"
        "src/tool/single"
        src/auth
        public
        ${GIO_2_INCLUDE_DIRS}
        ${GLIB_2_INCLUDE_DIRS}
        ${ZLOG_INCLUDE_DIRS}
        ${KIRAN_LOG_INCLUDE_DIRS}
        ${KIRAN_WIDGETS_INCLUDE_DIRS})

install(TARGETS ${PROJECT_NAME} DESTINATION ${SCREENSAVER_DIALOG_INSTALL})

install(FILES ${QM_FILES} DESTINATION ${SCREENSAVER_QM_INSTALL})

add_subdirectory(kiran-screensaver-checkpass)