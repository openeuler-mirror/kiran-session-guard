# kiran-session-guard

提供对于lightdm的显示管理器的登陆器和基于mate-screensaver的解锁框

## 使用

### 编译

- 安装依赖

  ```bash
  sudo yum install qt5-qtbase-devel qt5-linguist qt5-qtx11extras-devel kiranwidgets-qt5-devel kiran-log-qt5-devel libXtst-devel libX11-devel libXrandr-devel libXcursor-devel libXfixes-devel kiran-cc-daemon-devel glib2-devel kiran-biometrics-devel 
  ```

- 源码根目录下创建**build**目录

  ```bash
  mkdir build
  ```

- 进**build**目录，通过**cmake**生成**Makefile**

  ```bash
  cmake -DCMAKE_INSTALL_PREFIX=/usr ..
  ```

- 通过**make**进行编译

  ```bash
  make -j4
  ```

### 安装

- 在源码根目录下的**build**目录下，执行

  ```bash
  sudo make install
  ```

### 运行

- 重启**lightdm**服务即可运行**lightdm-kiran-greeter**

  ```bash
  sudo systemctl restart lightdm
  ```

- 重启mate-screensaver，再进行锁屏，即可运行kiran-screensaver-dialog

## 目录结构

源码根目录

├── checkpass-common  
├── lib  
│   ├── auth-proxy  
│   ├── common-widgets  
│   └── scaling-helper  
├── libexec  
│   └── session-guard-checkpass  
├── lightdm-greeter  
│   ├── kiran-cpanel-greeter  
└── screensaver-dialog



- checkpass-common

  封装 lib/auth-proxy/auth-pam和kiran-session-guard-checkpass之间通过管道

- lib/auth-proxy

  共用认证相关代码封装

- lib/common-widgets

  共用界面组件代码封装

- lib/scaling-helper

  界面缩放代码封装

- libexec/session-guard-checkpass

  AuthProxy/AuthPam中进行PAM认证的子进程

- lightdm-greeter

  lightdm登陆器lightdm-kiran-greeter的实现

- screensaver-dialog

  mate-screensaver的解锁框kiran-screensaver-dialog实现