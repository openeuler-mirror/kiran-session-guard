set (TARGET_NAME common-widgets)

file(GLOB_RECURSE COMMON_WIDGETS_SRC ./*.h ./*.cpp)
file(GLOB_RECURSE COMMON_WIDGETS_QRC resources/commonWidgets.qrc)

add_library(${TARGET_NAME} STATIC ${COMMON_WIDGETS_SRC} ${COMMON_WIDGETS_QRC})

target_include_directories(${TARGET_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${KLOG_QT5_INCLUDE_DIRS}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${KIRAN_STYLE_HELPER_INCLUDE_DIRS})

target_include_directories(${TARGET_NAME} INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${TARGET_NAME} PUBLIC
        Qt5::Core
        Qt5::Widgets
        ${KIRAN_STYLE_HELPER_LIBRARIES}
        ${KLOG_QT5_LIBRARIES})
