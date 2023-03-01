cmake_minimum_required(VERSION 3.0)

set(TARGET_NAME kiran-polkit-agent)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Widgets LinguistTools)
pkg_search_module(POLKIT_QT5 REQUIRED polkit-qt5-1)
pkg_search_module(KLOG_QT5 REQUIRED klog-qt5)
pkg_search_module(KIRAN_WIDGETS REQUIRED kiranwidgets-qt5)
pkg_search_module(KIRAN_STYLE_HELPER REQUIRED kiran-style-helper)

file(GLOB POLKIT_AGENT_SOURCE "*.cpp" "*.h" "*.ui")
set(POLKIT_AGENT_TRANSLATION ${CMAKE_SOURCE_DIR}/translations/kiran-polkit-agent.zh_CN.ts)
qt5_create_translation(POLKIT_AGENT_QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${POLKIT_AGENT_TRANSLATION})

add_executable(${TARGET_NAME} ${POLKIT_AGENT_SOURCE} ${POLKIT_AGENT_QM_FILES})

target_include_directories(${TARGET_NAME} PRIVATE
    src/
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KLOG_QT5_INCLUDE_DIRS}
    ${KIRAN_WIDGETS_INCLUDE_DIRS}
    ${POLKIT_QT5_INCLUDE_DIRS}
    ${KIRAN_STYLE_HELPER_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME}
    auth-proxy
    common-widgets
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    ${KLOG_QT5_LIBRARIES}
    ${KIRAN_WIDGETS_LIBRARIES}
    ${POLKIT_QT5_LIBRARIES}
    ${KIRAN_STYLE_HELPER_LIBRARIES})
message("POLKIT: ${POLKIT_QT5_LIBRARIES} ${POLKIT_QT5_INCLUDE_DIRS}")

install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBEXECDIR})

configure_file(${CMAKE_SOURCE_DIR}/data/kiran-polkit-agent.desktop.in 
               ${CMAKE_CURRENT_BINARY_DIR}/kiran-polkit-agent.desktop
               @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kiran-polkit-agent.desktop
        DESTINATION /${CMAKE_INSTALL_SYSCONFDIR}/xdg/autostart/ )