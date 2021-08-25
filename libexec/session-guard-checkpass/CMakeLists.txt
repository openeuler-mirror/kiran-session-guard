set(TARGET_NAME "kiran-session-guard-checkpass")

find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core)
pkg_search_module(KIRAN_LOG REQUIRED klog-qt5)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE SRC "*.cpp" "*.h")
file(GLOB_RECURSE CHECKPASS_COMMON "../../checkpass-common/*.cpp" "../../checkpass-common/*.h" )

add_executable(${TARGET_NAME} ${SRC} ${CHECKPASS_COMMON})

target_link_libraries(${TARGET_NAME} PUBLIC
        Qt5::Core
        ${KIRAN_LOG_LIBRARIES}
        pam)

target_include_directories(${TARGET_NAME} PRIVATE
        src
        ../../checkpass-common
        ${KIRAN_LOG_INCLUDE_DIRS})

install(TARGETS ${TARGET_NAME}
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBEXECDIR}/)