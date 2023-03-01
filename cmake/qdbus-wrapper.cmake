find_package(KiranDBusGenerate REQUIRED)

##kiran_qt5_add_dbus_interface_ex(DBUS_SRC_LIST                     生成的文件list
#        data/com.kylinsec.Kiran.SystemDaemon.Accounts.User.xml     xml文件路径
#        ksd_accounts_user_proxy                                    生成文件名
#        KSDAccountsUserProxy)                                      类名
#
function(KIRAN_QT5_ADD_DBUS_INTERFACE_EX _source _interface _basename _classname)
    set_source_files_properties(${_interface}
            PROPERTIES
            CLASSNAME ${_classname}
            NO_NAMESPACE true)
    kiran_qt5_add_dbus_interface( _source ${_interface} ${_basename})
    set(${_source} ${${_source}} PARENT_SCOPE)
endfunction()