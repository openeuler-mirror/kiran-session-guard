cmake_minimum_required(VERSION 3.5)

project(kiran-screensaver-dialog)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(GNUInstallDirs)
set(SCREENSAVER_DIALOG_INSTALL ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}/)
set(SCREENSAVER_QM_INSTALL ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/translations/)

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Widgets X11Extras DBus Network LinguistTools)
pkg_search_module(GIO_2 REQUIRED gio-2.0)
pkg_search_module(GLIB_2 REQUIRED glib-2.0)
pkg_search_module(X11 REQUIRED x11)
pkg_search_module(XCB REQUIRED xcb)
pkg_search_module(XRANDR REQUIRED xrandr)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE TOOL_SRC tool "tool/*")
file(GLOB_RECURSE SRC "src/*.cpp" "src/*.h" "src/*.ui")
file(GLOB_RECURSE RESOURCES "resources/*.qrc")

set( TS_FILES "translations/kiran-screensaver-dialog.zh_CN.ts" )
qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES} )

include_directories("${CMAKE_BINARY_DIR}" "src" "src/dbus-api-wrapper/" "tool" "tool/single")
add_executable(${PROJECT_NAME} ${TOOL_SRC} ${SRC} ${RESOURCES} ${QM_FILES})
target_link_libraries(${PROJECT_NAME}
        Qt5::Widgets
        Qt5::X11Extras
        Qt5::DBus
        Qt5::Network
        ${GIO_2_LIBRARIES}
        ${GLIB_2_LIBRARIES}
        ${X11_LIBRARIES}
        ${XRANDR_LIBRARIES}
        pam)
target_include_directories(${PROJECT_NAME} PRIVATE
        ${GIO_2_INCLUDE_DIRS}
        ${GLIB_2_INCLUDE_DIRS})

install(TARGETS ${PROJECT_NAME} DESTINATION ${SCREENSAVER_DIALOG_INSTALL})
install(FILES ${QM_FILES} DESTINATION ${SCREENSAVER_QM_INSTALL})