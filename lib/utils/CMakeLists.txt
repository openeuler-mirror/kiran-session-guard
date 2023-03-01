set(TARGET_NAME utils)

find_package(Qt5 COMPONENTS Widgets Core X11Extras DBus)
find_package(PkgConfig REQUIRED)
pkg_search_module(X11 REQUIRED x11)
pkg_search_module(XINPUT REQUIRED xi)
pkg_search_module(KLOG_QT5 REQUIRED klog-qt5)

file(GLOB UTILS_SRC "*.cpp" "*.h")

add_library(${TARGET_NAME} STATIC ${UTILS_SRC})

target_include_directories(${TARGET_NAME} PRIVATE
                    ${$CMAKE_CURRENT_SOURCE_DIR} 
                    ${PROJECT_SOURCE_DIR}/include
                    ${XINPUT_INCLUDE_DIRS}
                    ${KLOG_QT5_INCLUDE_DIRS})

target_include_directories(${TARGET_NAME} INTERFACE
                    ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${TARGET_NAME} 
                    dbus
                    Qt5::Widgets
                    Qt5::Core
                    Qt5::DBus
                    Qt5::X11Extras 
                    ${X11_LIBRARIES} 
                    ${XINPUT_LIBRARIES}
                    ${KLOG_QT5_LIBRARIES})
                    