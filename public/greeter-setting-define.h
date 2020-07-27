#ifndef __GREETER_SETTING_DEFINE_H__
#define __GREETER_SETTING_DEFINE_H__

#define  GREETER_SETTING_PATH                   "/etc/lightdm/lightdm-kiran-greeter.conf"
#define  GREETER_SETTING_GROUP                  "Greeter"

///背景配置
#define  KEY_BACKGROUND_URI                     "background-picture-uri"
#define  DEFAULT_BACKGROUND_URI                 ":/images/default_background.png"

///用户列表是否隐藏
#define  KEY_USER_LIST_HIDING                   "user-list-hiding"
#define  DEFAULT_USER_LIST_HIDING               false

///是否允许手动登录
#define  KEY_ENABLE_MANUAL_LOGIN                "enable-manual-login"
#define  DEFAULT_ENABLE_MANUAL_LOGIN            true

///是否开启缩放,支持三项配置 auto,enable,disable
#define  KEY_ENABLE_SCALING                     "enable-scaling"
#define  DEFAULT_ENABLE_SCALING                 "auto"

///缩放比例
#define  KEY_SCALE_FACTOR                       "scale-factor"
#define  DEFAULT_SCALE_FACTOR                   0.0

///Greeter消息显示间隔
#define  KEY_MESSAGE_INTERVAL                   "message-interval"
#define  DEFAULT_MESSAGE_INTERVAL               3
#endif

