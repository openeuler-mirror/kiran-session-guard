cmake_minimum_required(VERSION 3.2)

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

add_subdirectory(lightdm-greeter)
add_subdirectory(screensaver-dialog)
add_subdirectory(polkit-agent)