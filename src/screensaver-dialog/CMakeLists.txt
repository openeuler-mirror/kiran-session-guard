set(TARGET_NAME "kiran-screensaver-dialog")

pkg_search_module(GIO_2 REQUIRED gio-2.0)

set(SCREENSAVER_DIALOG_INSTALL
    ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/kiran-screensaver)
set(SCREENSAVER_QM_INSTALL
    ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${TARGET_NAME}/translations/
)

file(GLOB SCREENSAVER_DIALOG_SRC "*.cpp" "*.h" "*.ui")
file(GLOB CHECKPASS_SRC "checkpass-common/*.cpp" "checkpass-common/*.h")
set(SCREENSAVER_DIALOG_RESOURCE
    ${CMAKE_SOURCE_DIR}/resources/screensaver-dialog-resources.qrc)

set(SCREENSAVER_DIALOG_TRANSLATOR_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/lib/login-frame/
    ${PROJECT_SOURCE_DIR}/lib/auth-proxy/
    ${PROJECT_SOURCE_DIR}/lib/common-widgets/)

# 定义翻译源文件列表
set(SCREENSAVER_DIALOG_QM_FILES "")

foreach(LANG ${LANGUAGES})
  # ts 文件路径
  set(TS_FILE "${PROJECT_SOURCE_DIR}/translations/${TARGET_NAME}.${LANG}.ts")

  # 执行 lupdate 更新 ts 文件
  execute_process(
    COMMAND lupdate-qt5 -no-obsolete ${SCREENSAVER_DIALOG_TRANSLATOR_SOURCE}
            -ts ${TS_FILE})

  # https://bugreports.qt.io/browse/QTBUG-64317 5.9.4以下生成的qm文件不带${LANG}后缀
  # qt5_add_translation(QM_FILE ${TS_FILE})
  set(QM_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.${LANG}.qm")
  # 执行 lrelease 生成qm文件
  execute_process(COMMAND lrelease-qt5 ${TS_FILE} -qm ${QM_FILE})
  list(APPEND SCREENSAVER_DIALOG_QM_FILES ${QM_FILE})
  message(STATUS "QM_FILE: ${QM_FILE}")
endforeach()

add_library(
  ${TARGET_NAME} SHARED
  ${SCREENSAVER_DIALOG_SRC} ${CHECKPASS_SRC} ${SCREENSAVER_DIALOG_RESOURCE}
  ${SCREENSAVER_DIALOG_QM_FILES})

target_link_libraries(
  ${TARGET_NAME}
  PUBLIC dbus
         utils
         common-widgets
         auth-proxy
         login-frame
         pam
         Qt5::Widgets
         Qt5::X11Extras
         Qt5::DBus
         ${KLOG_QT5_LIBRARIES}
         ${KIRANWIDGETS_QT5_LIBRARIES}
         ${GIO_2_LIBRARIES})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_SOURCE_DIR}/include
          ${CMAKE_CURRENT_SOURCE_DIR}
          src
          src/dbus-api-wrapper
          src/widgets
          checkpass-common/
          ${KLOG_QT5_INCLUDE_DIRS}
          ${KIRANWIDGETS_QT5_INCLUDE_DIRS}
          ${GIO_2_INCLUDE_DIRS})

# 安装锁屏框插件
install(TARGETS ${TARGET_NAME} DESTINATION ${SCREENSAVER_DIALOG_INSTALL})

# 安装翻译文件
install(FILES ${SCREENSAVER_DIALOG_QM_FILES}
        DESTINATION ${SCREENSAVER_QM_INSTALL})

# 安装kiran-screensaver-dialog私有配置文件
configure_file(${CMAKE_SOURCE_DIR}/data/screensaver-dialog.ini.in
               ${CMAKE_CURRENT_BINARY_DIR}/screensaver-dialog.ini @ONLY)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/screensaver-dialog.ini
  DESTINATION
    ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${TARGET_NAME}/)

add_subdirectory(session-guard-checkpass)
