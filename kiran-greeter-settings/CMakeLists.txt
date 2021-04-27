cmake_minimum_required(VERSION 3.5)

project(kiran-greeter-settings)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(GNUInstallDirs)

# Qt依赖
find_package(Qt5 COMPONENTS
        Widgets
        DBus
        LinguistTools)

# 第三方依赖
find_package(PkgConfig REQUIRED)
pkg_search_module(GSETTINGS_QT REQUIRED gsettings-qt)
pkg_search_module(GLIB_2 REQUIRED glib-2.0)
pkg_search_module(KIRANWIDGETS_QT5 REQUIRED kiranwidgets-qt5)
pkg_search_module(ZLOG REQUIRED zlog)

# Qt编译设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 编译文件
file(GLOB_RECURSE PUBLIC_CPP "../public/*.cpp")
file(GLOB_RECURSE PUBLIC_HEADER "../public/*.h")

file(GLOB_RECURSE SETTINGS_SRC_CPP "src/*.cpp")
file(GLOB_RECURSE SETTINGS_SRC_HEADER "src/*.h")

file(GLOB_RECURSE SETTINGS_SRC_FORMS "src/*.ui")

# 资源文件
file(GLOB_RECURSE SETTINGS_RESOURCES "resources/*.qrc")

# 翻译
set(TS_FILES "translations/kiran-greeter-settings.zh_CN.ts")
qt5_create_translation(GREETER_SETTINS_QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

add_executable(${PROJECT_NAME}
        ${PUBLIC_CPP}
        ${PUBLIC_HEADER}
        ${SETTINGS_SRC_CPP}
        ${SETTINGS_SRC_HEADER}
        ${SETTINGS_SRC_FORMS}
        ${SETTINGS_RESOURCES}
        ${GREETER_SETTINS_QM_FILES})

include_directories("${CMAKE_BINARY_DIR}")

target_include_directories(${PROJECT_NAME} PRIVATE
        ../public
        ./src
        ${GSETTINGS_QT_INCLUDE_DIRS}
        ${GLIB_2_INCLUDE_DIRS}
        ${KIRANWIDGETS_QT5_INCLUDE_DIRS}
        ${ZLOG_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
        Qt5::Widgets
        Qt5::DBus
        ${GSETTINGS_QT_LIBRARIES}
        ${GLIB_2_LIBRARIES}
        ${KIRANWIDGETS_QT5_LIBRARIES}
        ${ZLOG_LIBRARIES})

# 安装
#安装kiran-greeter-settings
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
#翻译文件
install(FILES ${GREETER_SETTINS_QM_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/lightdm-kiran-greeter/translations/)
#桌面快捷方式
install(FILES ./config/kiran-greeter-settings.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/applications/)