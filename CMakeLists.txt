cmake_minimum_required(VERSION 3.5)

project(lightdm-kiran-greeter)

option(HAVE_BIOMETRICS_AUTH "have biometrics auth" OFF)
option(SHOW_VIRTUAL_KEYBOARD "show virtual keyboard" OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(GNUInstallDirs)

# add_definitions(-DTEST)
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

# Qt编译设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 编译文件
file(GLOB_RECURSE PUBLIC_CPP "public/*.cpp")
file(GLOB_RECURSE PUBLIC_HEADER "public/*.h")

file(GLOB_RECURSE SRC_CPP "src/*.cpp")
file(GLOB_RECURSE SRC_HEADER "src/*.h")

file(GLOB_RECURSE SRC_FORMS "src/*.ui")

# 资源文件
file(GLOB_RECURSE RESOURCES "resources/*.qrc")

# 翻译
set(TS_FILES "translations/lightdm-kiran-greeter.zh_CN.ts")
qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

include_directories(${CMAKE_BINARY_DIR})

add_executable(${PROJECT_NAME}
        ${PUBLIC_CPP}
        ${PUBLIC_HEADER}
        ${SRC_CPP}
        ${SRC_HEADER}
        ${SRC_FORMS}
        ${QM_FILES}
        ${RESOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
        ./public/
        ./src/
        ./src/auth/
        ${LIGHTDM_QT5_INCLUDE_DIRS}
        ${KLOG_QT5_INCLUDE_DIRS}
        ${KIRAN_CC_DAEMON_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
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

if (HAVE_BIOMETRICS_AUTH)
    add_definitions(-DBIOMETRICS_AUTH)
endif ()

if (SHOW_VIRTUAL_KEYBOARD)
    add_definitions(-DVIRTUAL_KEYBOARD)
endif()

#安装lightdm-kiran-greeter
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SBINDIR})

#安装翻译  /usr/share/lightdm-kiran-greeter/translations/
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/translations)

#安装lightdm greeter配置文件 /usr/share/lightdm/lightdm.conf.d/99-lightdm-kiran-greeter.conf
install(FILES config/99-lightdm-kiran-greeter.conf DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/lightdm/lightdm.conf.d/)

#安装xgreeter配置文件 /usr/share/xgreeters/
install(FILES config/lightdm-kiran-greeter.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/xgreeters/)

#安装greeter的私有的内置配置文件
install(FILES config/greeter.ini DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/)

#安装greeter zlog配置文件
install(FILES config/zlog.conf
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ WORLD_WRITE)

add_subdirectory(kiran-cpanel-greeter)