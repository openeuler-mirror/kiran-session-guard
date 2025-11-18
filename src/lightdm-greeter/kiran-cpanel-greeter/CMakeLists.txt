set(TARGET_NAME kiran-cpanel-greeter)

pkg_search_module(GSETTINGS_QT REQUIRED gsettings-qt)
pkg_search_module(GLIB_2 REQUIRED glib-2.0)

pkg_search_module(KIRAN_CONTROL_PANEL REQUIRED kiran-control-panel)
pkg_get_variable(CPANEL_PLUGIN_DIR kiran-control-panel plugin_location)
pkg_get_variable(CPANEL_DESKTOP_DIR kiran-control-panel plugin_desktop_location)

file(
  GLOB
  CPANEL_GREETER_SRC
  "../common/*.cpp"
  "../common/*.h"
  "./*.cpp"
  "./*.h"
  "./*.ui")
set(CPANEL_GREETER_RESOURCE
    ${CMAKE_SOURCE_DIR}/resources/kcp-greeter-resources.qrc)

# 定义翻译源文件列表
set(GREETER_SETTINS_QM_FILES "")

foreach(LANG ${LANGUAGES})
  # ts 文件路径
  set(TS_FILE "${PROJECT_SOURCE_DIR}/translations/${TARGET_NAME}.${LANG}.ts")

  # 执行 lupdate 更新 ts 文件
  execute_process(COMMAND lupdate-qt5 -no-obsolete ${CMAKE_CURRENT_SOURCE_DIR}
                          -ts ${TS_FILE})

  # https://bugreports.qt.io/browse/QTBUG-64317 5.9.4以下生成的qm文件不带${LANG}后缀
  # qt5_add_translation(QM_FILE ${TS_FILE})
  set(QM_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.${LANG}.qm")
  # 执行 lrelease 生成qm文件
  execute_process(COMMAND lrelease-qt5 ${TS_FILE} -qm ${QM_FILE})
  list(APPEND GREETER_SETTINS_QM_FILES ${QM_FILE})
  message(STATUS "QM_FILE: ${QM_FILE}")
endforeach()

add_library(
  ${TARGET_NAME} SHARED ${CPANEL_GREETER_SRC} ${CPANEL_GREETER_RESOURCE}
                        ${GREETER_SETTINS_QM_FILES})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ../public
          ./src
          ${PROJECT_SOURCE_DIR}/include
          ${GSETTINGS_QT_INCLUDE_DIRS}
          ${GLIB_2_INCLUDE_DIRS}
          ${KIRANWIDGETS_QT5_INCLUDE_DIRS}
          ${KLOG_QT5_INCLUDE_DIRS}
          ${KIRAN_CC_DAEMON_INCLUDE_DIRS}
          ${KIRAN_CONTROL_PANEL_INCLUDE_DIRS}
          ${KIRAN_STYLE_HELPER_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
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

# 安装 安装插件
install(TARGETS ${TARGET_NAME} DESTINATION ${CPANEL_PLUGIN_DIR}/)
# 翻译文件
install(
  FILES ${GREETER_SETTINS_QM_FILES}
  DESTINATION
    ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/lightdm-kiran-greeter/translations/
)
