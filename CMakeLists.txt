cmake_minimum_required(VERSION 3.5)

project(lightdm-kiran-greeter)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Qt依赖
find_package(Qt5 COMPONENTS
        Widgets
        X11Extras
        LinguistTools)

# 第三方依赖
find_package(PkgConfig REQUIRED)
pkg_search_module(LIGHTDM_QT5 REQUIRED liblightdm-qt5-3)
pkg_search_module(XTST REQUIRED xtst)
pkg_search_module(X11 REQUIRED x11)
pkg_search_module(XRANDR REQUIRED xrandr)
pkg_search_module(XCURSOR REQUIRED xcursor)
pkg_search_module(XFIXES REQUIRED xfixes)

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

add_executable(${PROJECT_NAME}
        ${PUBLIC_CPP}
        ${PUBLIC_HEADER}
        ${SRC_CPP}
        ${SRC_HEADER}
        ${SRC_FORMS}
        ${QM_FILES}
        ${RESOURCES})

include_directories("${CMAKE_BINARY_DIR}")

target_include_directories(${PROJECT_NAME} PRIVATE
        ./public/
        ./src/
        ./src/auth/
        ${LIGHTDM_QT5_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
        Qt5::Widgets
        Qt5::X11Extras
        pthread
        ${LIGHTDM_QT5_LIBRARIES}
        ${XTST_LIBRARIES}
        ${X11_LIBRARIES}
        ${XRANDR_LIBRARIES}
        ${XCURSOR_LIBRARIES}
        ${XFIXES_LIBRARIES})

# 安装
# SYSCONF_INSTALL_DIR,SHARE_INSTALL_PREFIX
# 为rpmbuild中定义的宏,若不经过rpmbuild进行编译,对这些值给默认初始化
if (NOT DEFINED SYSCONF_INSTALL_DIR)
    set(SYSCONF_INSTALL_DIR "/etc")
endif()
if (NOT DEFINED SHARE_INSTALL_PREFIX)
    set(SHARE_INSTALL_PREFIX "/usr/share")
endif()

#安装lightdm-kiran-greeter
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/)
#安装翻译  /usr/share/lightdm-kiran-greeter/translations/
install(FILES ${QM_FILES} DESTINATION ${SHARE_INSTALL_PREFIX}/lightdm-kiran-greeter/translations/)
#安装自身配置文件 /etc/lightdm/
install(FILES config/lightdm-kiran-greeter.conf DESTINATION ${SYSCONF_INSTALL_DIR}/lightdm/)
#安装lightdm greeter配置文件 /usr/share/lightdm/lightdm.conf.d/
install(FILES config/99-lightdm-kiran-greeter.conf DESTINATION ${SHARE_INSTALL_PREFIX}/lightdm/lightdm.conf.d/)
#安装xgreeter配置文件 /usr/share/xgreeters/
install(FILES config/lightdm-kiran-greeter.desktop DESTINATION ${SHARE_INSTALL_PREFIX}/xgreeters/)


add_subdirectory(kiran-greeter-settings)