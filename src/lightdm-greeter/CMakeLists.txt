cmake_minimum_required(VERSION 3.5)

set(TARGET_NAME lightdm-kiran-greeter)

# Qt依赖
find_package(Qt5 COMPONENTS
        Widgets
        DBus
        X11Extras
        DBus
        LinguistTools)

# 第三方依赖
find_package(PkgConfig REQUIRED)
pkg_search_module(LIGHTDM_QT5 REQUIRED liblightdm-qt5-3)
pkg_search_module(XTST REQUIRED xtst)
pkg_search_module(X11 REQUIRED x11)
pkg_search_module(XRANDR REQUIRED xrandr)
pkg_search_module(XCURSOR REQUIRED xcursor)
pkg_search_module(XFIXES REQUIRED xfixes)
pkg_search_module(KLOG_QT5 REQUIRED klog-qt5)
pkg_search_module(KIRAN_CC_DAEMON REQUIRED kiran-cc-daemon)
pkg_search_module(KIRAN_STYLE_HELPER REQUIRED kiran-style-helper)

# 编译文件
file(GLOB COMMON_SRC  "common/*.cpp" "common/*.h")
file(GLOB WIDGETS_SRC "widgets/*.cpp" "widgets/*.h" "widgets/*.ui")
file(GLOB GREETER_SRC "*.cpp" "*.h" "*.ui")
set(GREETER_RESOURCE ${CMAKE_SOURCE_DIR}/resources/greeter-resource.qrc)
set(GREETER_TRANSLATION ${CMAKE_SOURCE_DIR}/translations/lightdm-kiran-greeter.zh_CN.ts)
qt5_create_translation(GREETER_QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${GREETER_TRANSLATION})

add_executable(${TARGET_NAME}
        ${GREETER_SRC}
        ${COMMON_SRC}
        ${WIDGETS_SRC}
        ${GREETER_RESOURCE}
        ${GREETER_QM_FILES})

target_include_directories(${TARGET_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        ./common
        ./src/
        ${LIGHTDM_QT5_INCLUDE_DIRS}
        ${KLOG_QT5_INCLUDE_DIRS}
        ${KIRAN_CC_DAEMON_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME}
        common-widgets
        utils
        auth-proxy
        login-frame
        dbus
        Qt5::Widgets
        Qt5::DBus
        Qt5::X11Extras
        pthread
        ${LIGHTDM_QT5_LIBRARIES}
        ${XTST_LIBRARIES}
        ${X11_LIBRARIES}
        ${XRANDR_LIBRARIES}
        ${XCURSOR_LIBRARIES}
        ${XFIXES_LIBRARIES}
        ${KLOG_QT5_LIBRARIES}
        ${KIRAN_CC_DAEMON_LIBRARIES})
        
#安装lightdm-kiran-greeter
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SBINDIR})

#安装翻译  /usr/share/lightdm-kiran-greeter/translations/
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${TARGET_NAME}/translations)

#安装lightdm greeter配置文件 /usr/share/lightdm/lightdm.conf.d/99-lightdm-kiran-greeter.conf
install(FILES ${CMAKE_SOURCE_DIR}/data/99-lightdm-kiran-greeter.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/lightdm/lightdm.conf.d/)

#安装xgreeter配置文件 /usr/share/xgreeters/
install(FILES ${CMAKE_SOURCE_DIR}/data/lightdm-kiran-greeter.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/xgreeters/)

#通过编译选项生成greeter.ini
configure_file(${CMAKE_SOURCE_DIR}/data/greeter.ini.in ${CMAKE_CURRENT_BINARY_DIR}/greeter.ini @ONLY)

#安装greeter的私有的内置配置文件
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/greeter.ini
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${TARGET_NAME}/)

#安装greeter zlog配置文件
install(FILES ${CMAKE_SOURCE_DIR}/data/greeter-zlog.conf
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${TARGET_NAME}/
        RENAME zlog.conf
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ WORLD_WRITE)

#登录设置
add_subdirectory(kiran-cpanel-greeter)