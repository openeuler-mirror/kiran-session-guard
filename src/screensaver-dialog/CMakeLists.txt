cmake_minimum_required(VERSION 3.2)

set(TARGET_NAME "kiran-screensaver-dialog")

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

file(GLOB SCREENSAVER_DIALOG_SRC "*.cpp" "*.h" "*.ui")
file(GLOB CHECKPASS_SRC "checkpass-common/*.cpp" "checkpass-common/*.h")
set(SCREENSAVER_DIALOG_RESOURCE ${CMAKE_SOURCE_DIR}/resources/screensaver-dialog-resources.qrc)
set(SCREENSAVER_DIALOG_TRANSLATION ${CMAKE_SOURCE_DIR}/translations/kiran-screensaver-dialog.zh_CN.ts)
qt5_create_translation(SCREENSAVER_DIALOG_QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${SCREENSAVER_DIALOG_TRANSLATION})

add_library(${TARGET_NAME} SHARED ${SCREENSAVER_DIALOG_SRC} ${CHECKPASS_SRC} ${SCREENSAVER_DIALOG_RESOURCE} ${SCREENSAVER_DIALOG_QM_FILES})

target_link_libraries(${TARGET_NAME} PUBLIC
        dbus
        utils
        common-widgets
        auth-proxy
        login-frame
        pam
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
        ${KIRAN_WIDGETS_LIBRARIES})

target_include_directories(${TARGET_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        src
        src/dbus-api-wrapper
        src/widgets
        checkpass-common/
        ${GIO_2_INCLUDE_DIRS}
        ${GLIB_2_INCLUDE_DIRS}
        ${ZLOG_INCLUDE_DIRS}
        ${KIRAN_LOG_INCLUDE_DIRS}
        ${KIRAN_WIDGETS_INCLUDE_DIRS})

# 安装锁屏框插件
install(TARGETS ${TARGET_NAME} DESTINATION ${SCREENSAVER_DIALOG_INSTALL})

# 安装翻译文件
install(FILES ${QM_FILES} DESTINATION ${SCREENSAVER_QM_INSTALL})

# 安装kiran-screensaver-dialog私有配置文件
configure_file(${CMAKE_SOURCE_DIR}/data/screensaver.ini.in ${CMAKE_CURRENT_BINARY_DIR}/screensaver.ini @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/screensaver.ini DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/)