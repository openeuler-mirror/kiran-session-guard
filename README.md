# lightdm-kiran-greeter

## 编译
1.  安装编译依赖  
   `sudo yum install lightdm-qt5-devel qt5-qtbase-devel qt5-qtx11extras-devel libX11-devel libXtst-devel
libXrandr-devel libXcursor-devel
libXfixes-devel  gsettings-qt-devel  kiranwidgets-qt5-devel cmake qt5-linguist zlog-devel kiran-cc-daemon-devel`
2. **源码根目录**下创建**build**目录`mkdir build`
3. 进行**build**目录,执行`cmake -DCMAKE_INSTALL_PREFIX=/usr ..`生成**Makefile**
4. 执行`make`进行编译，生成可执行文件位于build下的**lightdm-kiran-greeter**

## 安装
1. 在**build**目录下执行`sudo make install`

## 卸载
1. 在**build**目录下执行`sudo make uninstall`

## 运行
编译安装运行后执行`systemctl restart lightdm`重启lightdm服务即可运行
