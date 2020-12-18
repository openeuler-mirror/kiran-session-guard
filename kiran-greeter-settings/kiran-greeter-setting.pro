#依赖的Qt模块
QT += core gui network dbus
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

message("greeter-settings:"$${DESTDIR})
#编译目标名
TARGET = kiran-greeter-settings

#生成Makefile模板
TEMPLATE = app

#宏定义
DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += TEST

#配置项
CONFIG += c++11
CONFIG +=link_pkgconfig

#头文件包含路径
INCLUDEPATH += ../public/

PKGCONFIG += glib-2.0
PKGCONFIG += gsettings-qt kiranwidgets-qt5

#源文件
SOURCES += \
    ../public/log.cpp \
    ../public/disabledeselectlistwidget.cpp\
    ../public/useravatarwidget.cpp \
    dbusapi.cpp \
    main.cpp \
    greetersetting.cpp \
    tabitem.cpp \
    logingreeterpreviewwidget.cpp \
    lightdmprefs.cpp \
    single/singleapplication.cpp \
    single/singleapplication_p.cpp

#头文件
HEADERS += \
    ../public/log.h \
    ../public/disabledeselectlistwidget.h\
    ../public/useravatarwidget.h \
    dbusapi.h \
    greetersetting.h \
    tabitem.h \
    logingreeterpreviewwidget.h \
    lightdmprefs.h \
    single/singleapplication.h \
    single/singleapplication_p.h

#界面文件
FORMS += \
    greetersetting.ui \
    tabitem.ui

#资源文件
RESOURCES += \
    image.qrc \
    themes.qrc

#翻译文件目录
TRANSLATIONS = \
    translations/kiran-greeter-settings.zh_CN.ts

#仅加载到QtCreator中
OTHER_FILES+= \
    translations/kiran-greeter-settings.zh_CN.ts \
    translations/kiran-greeter-settings.zh_CN.qm \
    config/kiran-greeter-settings.desktop \
    config/com.kiran.kiran-greeter-settings.policy \
    config/kiran-greeter-settings

LIBS += -lX11 -lXrandr
#安装选项
##翻译文件
target_translation.files = ./translations/kiran-greeter-settings.zh_CN.qm
target_translation.path = $$DESTDIR/usr/share/lightdm-kiran-greeter/translations/
##polkit提权策略
target_polkit.files = ./config/com.kiran.kiran-greeter-settings.policy
target_polkit.path = $$DESTDIR/usr/share/polkit-1/actions/
##桌面快捷方式
target_desktop.files = ./config/kiran-greeter-settings.desktop
target_desktop.path = $$DESTDIR/usr/share/applications/
##shell
target_shell.files = ./config/kiran-greeter-settings
target_shell.path = $$DESTDIR/usr/bin/
##可执行文件
target.path = $$DESTDIR/usr/sbin/

INSTALLS = target_translation target_polkit target_desktop target_shell target

