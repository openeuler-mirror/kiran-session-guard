#依赖的Qt模块
QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#编译目标名
TARGET = lightdm-kiran-greeter

#生成文件目录
DESTDIR = ./build

#生成Makefile模板
TEMPLATE = app

#宏定义
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += TEST

#配置项
CONFIG += c++11

#头文件包含路径
INCLUDEPATH += /usr/include/lightdm-qt5-3/

#源文件
SOURCES += \
    capslocksnoop.cpp \
    greeterbackground.cpp \
    greeterkeyboard.cpp \
    greeterlineedit.cpp \
    greeterloginwindow.cpp \
    greeterscreenmanager.cpp \
    greetersetting.cpp \
    log.cpp \
    main.cpp \
    useravatarwidget.cpp \
    userlistitem.cpp \
    userlistwidget.cpp \
    greetermenuitem.cpp

#头文件
HEADERS += \
    capslocksnoop.h \
    greeterbackground.h \
    greeterkeyboard.h \
    greeterlineedit.h \
    greeterloginwindow.h \
    greeterscreenmanager.h \
    greetersetting.h \
    log.h \
    userlistitem.h \
    userlistwidget.h \
    useravatarwidget.h \
    userinfo.h \
    greetermenuitem.h

#界面文件
FORMS += \
    greeterlineedit.ui \
    greeterloginwindow.ui \
    userlistwidget.ui \
    userlistitem.ui

#依赖库
LIBS += \
    -llightdm-qt5-3 \
    -lX11 \
    -lXtst

#资源文件
RESOURCES += \
    image.qrc \
    themes.qrc

#其他文件 (只是为了QtCreator能加载)
OTHER_FILES += \
    README.md \
    translations/lightdm-kiran-greeter.zh_CN.ts \
    config/lightdm-kiran-greeter.conf \
    config/99-lightdm-kiran-greeter.conf

#翻译文件目录
TRANSLATIONS = \
    translations/lightdm-kiran-greeter.zh_CN.ts

#安装选项
target_translation.files = ./translations/lightdm-kiran-greeter.zh_CN.qm
target_translation.path = /usr/share/lightdm-kiran-greeter/translations/

target_config.files = ./config/lightdm-kiran-greeter.conf
target_config.path = /etc/lightdm/

target_sysconfig.files = ./config/99-lightdm-kiran-greeter.conf
target_sysconfig.path =  /usr/share/lightdm/lightdm.conf.d/

target.path = /usr/sbin/
INSTALLS += target target_translation target_config target_sysconfig
