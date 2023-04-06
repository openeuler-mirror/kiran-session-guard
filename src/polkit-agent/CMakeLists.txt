cmake_minimum_required(VERSION 3.0)

set(TARGET_NAME kiran-polkit-agent)

set(POLKIT_AGENT_QM_INSTALL ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${TARGET_NAME}/translations/)

pkg_search_module(POLKIT_QT5 REQUIRED polkit-qt5-1)

file(GLOB POLKIT_AGENT_SOURCE "*.cpp" "*.h" "*.ui")

set(POLKIT_AGENT_TRANSLATOR_SOURCE      
        ${CMAKE_CURRENT_SOURCE_DIR}                
        ${PROJECT_SOURCE_DIR}/lib/common-widgets/
        ${PROJECT_SOURCE_DIR}/lib/auth-proxy/)
qt5_create_translation(POLKIT_AGENT_QM_FILES   
                       ${POLKIT_AGENT_TRANSLATOR_SOURCE}
                       ${CMAKE_SOURCE_DIR}/translations/kiran-polkit-agent.zh_CN.ts
                       OPTIONS -I ${POLKIT_AGENT_TRANSLATOR_SOURCE})

add_executable(${TARGET_NAME} 
               ${POLKIT_AGENT_SOURCE} 
               ${POLKIT_AGENT_QM_FILES})

target_include_directories(${TARGET_NAME} PRIVATE
    src/
    ${PROJECT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_BINARY_DIR}
    ${KLOG_QT5_INCLUDE_DIRS}
    ${KIRANWIDGETS_QT5_INCLUDE_DIRS}
    ${POLKIT_QT5_INCLUDE_DIRS}
    ${KIRAN_STYLE_HELPER_INCLUDE_DIRS})

target_link_libraries(${TARGET_NAME}
    auth-proxy
    common-widgets
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    ${KLOG_QT5_LIBRARIES}
    ${KIRANWIDGETS_QT5_LIBRARIES}
    ${POLKIT_QT5_LIBRARIES}
    ${KIRAN_STYLE_HELPER_LIBRARIES})
    
install(TARGETS ${TARGET_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBEXECDIR})

configure_file(${CMAKE_SOURCE_DIR}/data/kiran-polkit-agent.desktop.in 
               ${CMAKE_CURRENT_BINARY_DIR}/kiran-polkit-agent.desktop
               @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kiran-polkit-agent.desktop
        DESTINATION /${CMAKE_INSTALL_SYSCONFDIR}/xdg/autostart/ )

install(FILES ${POLKIT_AGENT_QM_FILES} DESTINATION ${POLKIT_AGENT_QM_INSTALL})
