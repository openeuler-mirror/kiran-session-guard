TEMPLATE = subdirs
CONFIG += ordered

message("lightdm-kiran-greeter:"$${DESTDIR})

SUBDIRS += \
    kiran-greeter \
    kiran-greeter-setting

#其他文件 (只是为了QtCreator能加载)
OTHER_FILES += \
    README.md
