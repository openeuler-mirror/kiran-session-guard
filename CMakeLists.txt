cmake_minimum_required(VERSION 3.5)

project(kiran-session-guard)

include(GNUInstallDirs)
include_directories(/usr/include/)

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
#是否使用FullName用于替代用户名
set(SHOW_FULLNAME false CACHE BOOL "Whether to use FullName to replace the username")

add_subdirectory(./lib)
add_subdirectory(./libexec)
add_subdirectory(./lightdm-greeter)
add_subdirectory(./screensaver-dialog)
