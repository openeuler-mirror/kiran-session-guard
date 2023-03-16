set(TARGET_NAME dbus)

set(DBUS_SRC_LIST "")

kiran_qt5_add_dbus_interface_ex(KSD_ACCOUNTS_SRC
        com.kylinsec.Kiran.SystemDaemon.Accounts.xml
        ksd_accounts_proxy
        KSDAccountsProxy)
list(APPEND DBUS_SRC_LIST ${KSD_ACCOUNTS_SRC})

kiran_qt5_add_dbus_interface_ex(KSD_ACCOUNTS_USER_SRC
        com.kylinsec.Kiran.SystemDaemon.Accounts.User.xml
        ksd_accounts_user_proxy
        KSDAccountsUserProxy)
list(APPEND DBUS_SRC_LIST ${KSD_ACCOUNTS_USER_SRC})

kiran_qt5_add_dbus_interface_ex(KSD_GREETER_SRC
        com.kylinsec.Kiran.SystemDaemon.Greeter.xml
        ksd_greeter_proxy
        KSDGreeterProxy)
list(APPEND DBUS_SRC_LIST ${KSD_GREETER_SRC})

# kiran_qt5_add_dbus_interface_ex(FD_ACCOUNT_SRC
#         org.freedesktop.Accounts.xml
#         fd_accounts_proxy
#         FDAccountsProxy)
# list(APPEND DBUS_SRC_LIST ${FD_ACCOUNT_SRC})

# kiran_qt5_add_dbus_interface_ex(FD_ACCOUNT_USER_SRC
#         org.freedesktop.Accounts.User.xml
#         fd_accounts_user_proxy
#         FDAccountsUserProxy)
# list(APPEND DBUS_SRC_LIST ${FD_ACCOUNT_USER_SRC})

kiran_qt5_add_dbus_interface_ex(GNOME_SM_SRC
        org.gnome.SessionManager.xml
        gnome_sm_proxy
        GnomeSMProxy)
list(APPEND DBUS_SRC_LIST ${GNOME_SM_SRC})

file(GLOB DBUS_SRC "*.cpp" "*.h")
add_library(${TARGET_NAME} STATIC ${DBUS_SRC_LIST} ${DBUS_SRC})

target_include_directories(${TARGET_NAME} PRIVATE
                    ${PROJECT_SOURCE_DIR}/include
                    {$CMAKE_CURRENT_SOURCE_DIR} 
                    ${KLOG_QT5_INCLUDE_DIRS})

target_include_directories(${TARGET_NAME} INTERFACE
                        ${CMAKE_CURRENT_SOURCE_DIR}
                        ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${TARGET_NAME} Qt5::Core Qt5::DBus ${KLOG_QT5_LIBRARIES})