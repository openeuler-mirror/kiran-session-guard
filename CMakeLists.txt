cmake_minimum_required(VERSION 3.5)

project(kiran-session-guard)

include(GNUInstallDirs)

add_subdirectory(./lib)
add_subdirectory(./libexec)
add_subdirectory(./lightdm-greeter)
add_subdirectory(./screensaver-dialog)