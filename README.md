# kiran-screensaver-dialog

## 编译
1. 安装编译依赖 
    `sudo yum install qt5-qtbase qt5-qtbase-devel qt5-qtx11extras qt5-qtx11extras-devel libX11 libX11-devel glib2 glib2-devel pam pam-dev`
2. **源码根目录**下创建**build**目录`mkdir build`
3. 进行**build**目录,执行`qmake-qt5 ..`生成**Makefile**
4. 执行`make`进行编译，生成可执行文件位于build下的**kiran-screensaver-dialog**
   
## 安装
1. 在**build**目录下执行`sudo make install`
2. gsettings set org.mate.screensaver lock-dialog-command "kiran-screensaver-dialog"
3. gsettings set org.mate.screensaver lock-dialog-fullscreen true

## 卸载
1. 在**build**目录下执行`sudo make uninstall`
2. gsettings set org.mate.screensaver lock-dialog-command "mate-screensaver-dialog"
3. gsettings set org.mate.screensaver lock-dialog-fullscreen false
   
## 运行
运行`mate-screensaver-command -l`即可锁定窗口