set(TARGET_NAME lightdm-kiran-greeter)

pkg_search_module(LIGHTDM_QT5 REQUIRED liblightdm-qt5-3)
pkg_search_module(XCB REQUIRED xcb)
pkg_search_module(XCB_RANDR REQUIRED xcb-randr)

file(GLOB COMMON_SRC "common/*.cpp" "common/*.h")
file(GLOB WIDGETS_SRC "widgets/*.cpp" "widgets/*.h" "widgets/*.ui")
file(GLOB GREETER_SRC "*.cpp" "*.h" "*.ui")
set(GREETER_RESOURCE ${CMAKE_SOURCE_DIR}/resources/greeter-resource.qrc)

set(LIGHTDM_GREETER_TRANSLATOR_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/lib/login-frame/
    ${PROJECT_SOURCE_DIR}/lib/auth-proxy/
    ${PROJECT_SOURCE_DIR}/lib/common-widgets/)

# 定义翻译源文件列表
set(GREETER_QM_FILES "")

foreach(LANG ${LANGUAGES})
  # ts 文件路径
  set(TS_FILE "${PROJECT_SOURCE_DIR}/translations/${TARGET_NAME}.${LANG}.ts")

  # 执行 lupdate 更新 ts 文件
  execute_process(COMMAND lupdate-qt5 -no-obsolete
                          ${LIGHTDM_GREETER_TRANSLATOR_SOURCE} -ts ${TS_FILE})

  # https://bugreports.qt.io/browse/QTBUG-64317 5.9.4以下生成的qm文件不带${LANG}后缀
  # qt5_add_translation(QM_FILE ${TS_FILE})
  set(QM_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.${LANG}.qm")
  # 执行 lrelease 生成qm文件
  execute_process(COMMAND lrelease-qt5 ${TS_FILE} -qm ${QM_FILE})
  list(APPEND GREETER_QM_FILES ${QM_FILE})
  message(STATUS "QM_FILE: ${QM_FILE}")
endforeach()

add_executable(${TARGET_NAME} ${GREETER_SRC} ${COMMON_SRC} ${WIDGETS_SRC}
                              ${GREETER_RESOURCE} ${GREETER_QM_FILES})

target_include_directories(
  ${TARGET_NAME}
  PRIVATE ${PROJECT_SOURCE_DIR}/include
          ${CMAKE_CURRENT_SOURCE_DIR}
          ./common
          ./src/
          ${XCB_INCLUDE_DIRS}
          ${XCB_RANDR_INCLUDE_DIRS}
          ${LIGHTDM_QT5_INCLUDE_DIRS}
          ${KLOG_QT5_INCLUDE_DIRS})

target_link_libraries(
  ${TARGET_NAME}
  common-widgets
  utils
  auth-proxy
  login-frame
  dbus
  Qt5::Widgets
  Qt5::DBus
  Qt5::X11Extras
  pthread
  ${XCB_LIBRARIES}
  ${XCB_RANDR_LIBRARIES}
  ${LIGHTDM_QT5_LIBRARIES}
  ${KLOG_QT5_LIBRARIES})
        
if(KIRAN_CC_DAEMON_FOUND)
    target_compile_definitions(${TARGET_NAME} PRIVATE HAVE_KIRAN_CC_DAEMON)
    target_include_directories(${TARGET_NAME} PRIVATE ${KIRAN_CC_DAEMON_INCLUDE_DIRS})
    target_link_libraries(${TARGET_NAME} PRIVATE ${KIRAN_CC_DAEMON_LIBRARIES})
endif()

# 安装lightdm-kiran-greeter
install(TARGETS ${TARGET_NAME}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SBINDIR})

# 安装翻译  /usr/share/lightdm-kiran-greeter/translations/
install(
  FILES ${GREETER_QM_FILES}
  DESTINATION
    ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${TARGET_NAME}/translations
)

# 安装lightdm greeter配置文件
# /usr/share/lightdm/lightdm.conf.d/99-lightdm-kiran-greeter.conf
install(
  FILES ${CMAKE_SOURCE_DIR}/data/99-lightdm-kiran-greeter.conf
  DESTINATION
    ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/lightdm/lightdm.conf.d/
)

# 安装xgreeter配置文件 /usr/share/xgreeters/
install(
  FILES ${CMAKE_SOURCE_DIR}/data/lightdm-kiran-greeter.desktop
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/xgreeters/)

# 通过编译选项生成greeter.ini
configure_file(${CMAKE_SOURCE_DIR}/data/greeter.ini.in
               ${CMAKE_CURRENT_BINARY_DIR}/greeter.ini @ONLY)

# 安装greeter的私有的内置配置文件
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/greeter.ini
  DESTINATION
    ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${TARGET_NAME}/)

# 安装greeter zlog配置文件
install(
  FILES ${CMAKE_SOURCE_DIR}/data/greeter-zlog.conf
  DESTINATION
    ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${TARGET_NAME}/
  RENAME zlog.conf
  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ
              WORLD_WRITE)

# 登录设置
if(ENABLE_KIRAN_CPANEL_GREETER)
  add_subdirectory(kiran-cpanel-greeter)
endif()
