set(TARGET_NAME kiran-cpanel-greeter)

pkg_search_module(GSETTINGS_QT REQUIRED gsettings-qt)
pkg_search_module(GLIB_2 REQUIRED glib-2.0)

pkg_search_module(KIRAN_CONTROL_PANEL REQUIRED kiran-control-panel)
pkg_get_variable(CPANEL_PLUGIN_DIR kiran-control-panel plugin_location)
pkg_get_variable(CPANEL_DESKTOP_DIR kiran-control-panel plugin_desktop_location)

file(GLOB CPANEL_GREETER_SRC "../common/*.cpp" "../common/*.h" "./*.cpp" "./*.h" "./*.ui")
set(CPANEL_GREETER_RESOURCE ${CMAKE_SOURCE_DIR}/resources/kcp-greeter-resources.qrc)
file(GLOB CPANEL_GREETER_TRANSLATION ${CMAKE_SOURCE_DIR}/translations/kiran-cpanel-greeter.*.ts)
qt5_create_translation(GREETER_SETTINS_QM_FILES 
        ${CMAKE_CURRENT_SOURCE_DIR} 
        ${CPANEL_GREETER_TRANSLATION}
        OPTIONS -I ${CMAKE_CURRENT_SOURCE_DIR} -I ${CMAKE_CURRENT_BINARY_DIR} )

add_library(${TARGET_NAME} SHARED
            ${CPANEL_GREETER_SRC}
            ${CPANEL_GREETER_RESOURCE}
            ${GREETER_SETTINS_QM_FILES})

target_include_directories(${TARGET_NAME} PRIVATE
        ../public
        ./src
        ${PROJECT_SOURCE_DIR}/include
        ${GSETTINGS_QT_INCLUDE_DIRS}
        ${GLIB_2_INCLUDE_DIRS}
        ${KIRANWIDGETS_QT5_INCLUDE_DIRS}
        ${KLOG_QT5_INCLUDE_DIRS}
        ${KIRAN_CC_DAEMON_INCLUDE_DIRS}
        ${KIRAN_CONTROL_PANEL_INCLUDE_DIRS}
        ${KIRAN_STYLE_HELPER_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME}
        Qt5::Widgets
        Qt5::DBus
        dbus
        utils
        ${GSETTINGS_QT_LIBRARIES}
        ${GLIB_2_LIBRARIES}
        ${KIRANWIDGETS_QT5_LIBRARIES}
        ${KLOG_QT5_LIBRARIES}
        ${KIRAN_CC_DAEMON_LIBRARIES}
        ${KIRAN_STYLE_HELPER_LIBRARIES})

# 安装
#安装插件desktop文件
configure_file(${CMAKE_SOURCE_DIR}/data/kiran-cpanel-greeter.desktop.in ${CMAKE_CURRENT_BINARY_DIR}/kiran-cpanel-greeter.desktop @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kiran-cpanel-greeter.desktop DESTINATION ${CPANEL_DESKTOP_DIR}/)

#安装插件
install(TARGETS ${TARGET_NAME} DESTINATION ${CPANEL_PLUGIN_DIR}/)
#翻译文件
install(FILES ${GREETER_SETTINS_QM_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/lightdm-kiran-greeter/translations/)
