set(TARGET_NAME "kiran-screensaver-dialog")

pkg_search_module(GIO_2 REQUIRED gio-2.0)

set(SCREENSAVER_DIALOG_INSTALL ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/kiran-screensaver)
set(SCREENSAVER_QM_INSTALL ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${TARGET_NAME}/translations/)

file(GLOB SCREENSAVER_DIALOG_SRC "*.cpp" "*.h" "*.ui")
file(GLOB CHECKPASS_SRC "checkpass-common/*.cpp" "checkpass-common/*.h")
set(SCREENSAVER_DIALOG_RESOURCE ${CMAKE_SOURCE_DIR}/resources/screensaver-dialog-resources.qrc)

set(SCREENSAVER_DIALOG_TRANSLATOR_SOURCE      
        ${CMAKE_CURRENT_SOURCE_DIR}                
        ${PROJECT_SOURCE_DIR}/lib/login-frame/
        ${PROJECT_SOURCE_DIR}/lib/auth-proxy/)
file(GLOB SCREENSAVER_DIALOG_TRANSLATION ${CMAKE_SOURCE_DIR}/translations/kiran-screensaver-dialog*.ts)
qt5_create_translation(SCREENSAVER_DIALOG_QM_FILES   
                       ${SCREENSAVER_DIALOG_TRANSLATOR_SOURCE}
                       ${SCREENSAVER_DIALOG_TRANSLATION}
                       OPTIONS -I ${SCREENSAVER_DIALOG_TRANSLATOR_SOURCE})
                       
add_library(${TARGET_NAME} SHARED 
            ${SCREENSAVER_DIALOG_SRC} 
            ${CHECKPASS_SRC} 
            ${SCREENSAVER_DIALOG_RESOURCE} 
            ${SCREENSAVER_DIALOG_QM_FILES})

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
        ${KLOG_QT5_LIBRARIES}
        ${KIRANWIDGETS_QT5_LIBRARIES}
        ${GIO_2_LIBRARIES})

target_include_directories(${TARGET_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/include
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
install(FILES ${SCREENSAVER_DIALOG_QM_FILES} DESTINATION ${SCREENSAVER_QM_INSTALL})

# 安装kiran-screensaver-dialog私有配置文件
configure_file(${CMAKE_SOURCE_DIR}/data/screensaver-dialog.ini.in ${CMAKE_CURRENT_BINARY_DIR}/screensaver-dialog.ini @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/screensaver-dialog.ini DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${TARGET_NAME}/)

add_subdirectory(session-guard-checkpass)
