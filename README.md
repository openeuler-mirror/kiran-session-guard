一、环境搭建：
    1.在Linux环境下安装GTK+3以及相关文档：sudo yum install gtk3-devel.x86_64 gtk3_devel_docs.x86_64
    2.在Linux环境下安装Glade：sudo yum install glade
    （glade是GTK＋的界面辅助设计工具，可以通过拖放控件的方式快速设计出用户界面，
            这样的优势在于在设计的同时能直观地看到界面上的控件，并且可以随时调整界面上的设计。）
            
二、源代码位置：
    1.锁屏界面：1.3.3-6-登录锁屏界面/2.lock_screen/lock_screen.c
    2.登陆界面：1.3.3-6-登录锁屏界面/1.login/login.c
    
三、源码编译运行：
    1.编译：锁屏界面：gcc -Wall -g  -o lock_screen lock_screen.c `pkg-config --cflags --libs gtk+-3.0`
            登陆界面：gcc -Wall -g  -o login login.c `pkg-config --cflags --libs gtk+-3.0`
    2.运行：锁屏界面：./lock_screen
            登陆界面：./login