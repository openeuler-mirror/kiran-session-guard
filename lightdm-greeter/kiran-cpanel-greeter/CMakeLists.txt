cmake_minimum_required(VERSION 3.5)

project(kiran-cpanel-greeter)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(GNUInstallDirs)

# Qt依赖
find_package(Qt5 COMPONENTS
        Widgets
        DBus
        LinguistTools)

# 第三方依赖
find_package(PkgConfig REQUIRED)
pkg_search_module(GSETTINGS_QT REQUIRED gsettings-qt)
pkg_search_module(GLIB_2 REQUIRED glib-2.0)
pkg_search_module(KIRANWIDGETS_QT5 REQUIRED kiranwidgets-qt5)
pkg_search_module(KLOG_QT5 REQUIRED klog-qt5)
pkg_search_module(KIRAN_CC_DAEMON REQUIRED kiran-cc-daemon)

pkg_search_module(KIRAN_CONTROL_PANEL REQUIRED kiran-control-panel)
pkg_get_variable(CPANEL_PLUGIN_DIR kiran-control-panel plugin_location)
pkg_get_variable(CPANEL_DESKTOP_DIR kiran-control-panel plugin_desktop_location)

# Qt编译设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 编译文件
file(GLOB_RECURSE PUBLIC_CPP "../public/*.cpp")
file(GLOB_RECURSE PUBLIC_HEADER "../public/*.h")

file(GLOB_RECURSE SETTINGS_SRC_CPP "src/*.cpp")
file(GLOB_RECURSE SETTINGS_SRC_HEADER "src/*.h")

file(GLOB_RECURSE SETTINGS_SRC_FORMS "src/*.ui")

# 资源文件
file(GLOB_RECURSE SETTINGS_RESOURCES "resources/*.qrc")

# 翻译
set(GREETER_SETTINGS_TS_FILES "translations/kiran-cpanel-greeter.zh_CN.ts")
qt5_create_translation(GREETER_SETTINS_QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${GREETER_SETTINGS_TS_FILES})

add_library(${PROJECT_NAME} SHARED
        ${PUBLIC_CPP}
        ${PUBLIC_HEADER}
        ${SETTINGS_SRC_CPP}
        ${SETTINGS_SRC_HEADER}
        ${SETTINGS_SRC_FORMS}
        ${SETTINGS_RESOURCES}
        ${GREETER_COMMON_SRC}
        ${GREETER_SETTINS_QM_FILES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ../public
        ./src
        ${GSETTINGS_QT_INCLUDE_DIRS}
        ${GLIB_2_INCLUDE_DIRS}
        ${KIRANWIDGETS_QT5_INCLUDE_DIRS}
        ${KLOG_QT5_INCLUDE_DIRS}
        ${KIRAN_CC_DAEMON_INCLUDE_DIRS}
        ${KIRAN_CONTROL_PANEL_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
        Qt5::Widgets
        Qt5::DBus
        ${GSETTINGS_QT_LIBRARIES}
        ${GLIB_2_LIBRARIES}
        ${KIRANWIDGETS_QT5_LIBRARIES}
        ${KLOG_QT5_LIBRARIES}
        ${KIRAN_CC_DAEMON_LIBRARIES})

# 安装
#安装插件desktop文件
configure_file(data/kiran-cpanel-greeter.desktop.in ${CMAKE_BINARY_DIR}/kiran-cpanel-greeter.desktop @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/kiran-cpanel-greeter.desktop DESTINATION ${CPANEL_DESKTOP_DIR}/)

SET(link_source ${CPANEL_DESKTOP_DIR}/${PROJECT_NAME}.desktop)
SET(link_target ${CMAKE_BINARY_DIR}/${PROJECT_NAME}-link.desktop)
add_custom_target(kcp-applications-desktop ALL COMMAND ln -sf ${link_source} ${link_target})
install(FILES ${link_target}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/applications/
        RENAME ${PROJECT_NAME}.desktop)

#安装插件
install(TARGETS ${PROJECT_NAME} DESTINATION ${CPANEL_PLUGIN_DIR}/)
#翻译文件
install(FILES ${GREETER_SETTINS_QM_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/lightdm-kiran-greeter/translations/)
#插件图标
set(KCP_ICON_INSTALL_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor)
install(FILES data/kcp-greeter-24.png DESTINATION ${KCP_ICON_INSTALL_PATH}/24x24/apps RENAME kcp-greeter.png)
install(FILES data/kcp-greeter-32.png DESTINATION ${KCP_ICON_INSTALL_PATH}/32x32/apps RENAME kcp-greeter.png)
install(FILES data/kcp-greeter-48.png DESTINATION ${KCP_ICON_INSTALL_PATH}/48x48/apps RENAME kcp-greeter.png)
install(FILES data/kcp-greeter-64.png DESTINATION ${KCP_ICON_INSTALL_PATH}/64x64/apps RENAME kcp-greeter.png)
install(FILES data/kcp-greeter-96.png DESTINATION ${KCP_ICON_INSTALL_PATH}/96x96/apps RENAME kcp-greeter.png)