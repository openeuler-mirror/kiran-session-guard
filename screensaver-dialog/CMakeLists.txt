cmake_minimum_required(VERSION 3.5)
project(kiran-screensaver-dialog)

#构建为kiran-screensaver插件
set(BUILD_SCREENSAVER_PLUGIN true CACHE BOOL "build libray as kiran-screensaver plugin")
configure_file(config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(GNUInstallDirs)
set(SCREENSAVER_DIALOG_INSTALL ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/kiran-screensaver)
set(SCREENSAVER_QM_INSTALL ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/translations/)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Widgets X11Extras DBus Network LinguistTools)

pkg_search_module(GIO_2 REQUIRED gio-2.0)
pkg_search_module(GLIB_2 REQUIRED glib-2.0)
pkg_search_module(X11 REQUIRED x11)
pkg_search_module(XCB REQUIRED xcb)
pkg_search_module(XRANDR REQUIRED xrandr)
pkg_search_module(KIRAN_LOG REQUIRED klog-qt5)
pkg_search_module(KIRAN_WIDGETS REQUIRED kiranwidgets-qt5)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE SRC "src/*.cpp" "src/*.h" "src/*.ui")
# 资源文件
file(GLOB_RECURSE RESOURCES "resources/*.qrc")

# 翻译文件
set( TS_FILES "translations/kiran-screensaver-dialog.zh_CN.ts" )
file(GLOB_RECURSE AUTH_PROXY_SRC ../lib/auth-proxy/*.cpp ../lib/auth-proxy/*.h ../lib/auth-proxy/*.ui)
qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES} ${AUTH_PROXY_SRC} )

if(BUILD_SCREENSAVER_PLUGIN)
    add_library(${PROJECT_NAME} SHARED ${SRC} ${RESOURCES} ${QM_FILES})
else()
    add_executable(${PROJECT_NAME} ${SRC} ${RESOURCES} ${QM_FILES})
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC
        common
        common-widgets
        scaling-helper
        auth-proxy
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
        ${CMAKE_CURRENT_SOURCE_DIR}
        src
        src/dbus-api-wrapper
        src/widgets
        ${GIO_2_INCLUDE_DIRS}
        ${GLIB_2_INCLUDE_DIRS}
        ${ZLOG_INCLUDE_DIRS}
        ${KIRAN_LOG_INCLUDE_DIRS}
        ${KIRAN_WIDGETS_INCLUDE_DIRS})

# 安装锁屏框插件
install(TARGETS ${PROJECT_NAME} DESTINATION ${SCREENSAVER_DIALOG_INSTALL})

# 安装翻译文件
install(FILES ${QM_FILES} DESTINATION ${SCREENSAVER_QM_INSTALL})

# 安装kiran-screensaver-dialog私有配置文件
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/screensaver.ini.in
        ${CMAKE_BINARY_DIR}/screensaver.ini @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/screensaver.ini
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/)