cmake_minimum_required(VERSION 3.5)

project(kiran-session-guard)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(GNUInstallDirs)
include(qdbus-wrapper)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#登录中隐藏的用户列表，以‘，’分割
set(HIDDEN_USER "" CACHE STRING "hidden user list,separated by commas")
#是否显示关机选项
set(CAN_POWER_OFF true CACHE BOOL "shutdown button display")
#是否显示重启选项
set(CAN_REBOOT true CACHE BOOL "reboot button display")
#是否显示挂起选项
set(CAN_SUSPEND true CACHE BOOL "suspend button display")
#是否显示休眠选项
set(CAN_HIBERNATE true CACHE BOOL "hibernate button display")
#NumLock初始化状态,是否点亮NumLock，true点亮NumLock,false不处理
set(NUMLOCK_INIT_STATE true CACHE BOOL "NumLock initialization status")
#是否使用FullName用于替代用户名
set(SHOW_FULLNAME false CACHE BOOL "Whether to use FullName to replace the username")

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Widgets X11Extras DBus LinguistTools)
pkg_search_module(KLOG_QT5 REQUIRED klog-qt5)
pkg_search_module(KIRAN_STYLE_HELPER REQUIRED kiran-style-helper)
pkg_search_module(KIRAN_CC_DAEMON REQUIRED kiran-cc-daemon)
pkg_search_module(KIRANWIDGETS_QT5 REQUIRED kiranwidgets-qt5)

add_subdirectory(./lib)
add_subdirectory(./src)