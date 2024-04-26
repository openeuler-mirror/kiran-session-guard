# kiran-session-guard

Provides a lander for lightdm's display manager and an unlock box based on mate-screensaver 

## Use

### Compilation

- installation dependency 

  ```
  sudo yum install qt5-qtbase-devel qt5-linguist qt5-qtx11extras-devel kiranwidgets-qt5-devel kiran-log-qt5-devel kiran-cc-daemon-devel kiran-biometrics-devel kiran-authentication-service-devel kiran-control-panel-devel libXtst-devel libX11-devel libXrandr-devel libXcursor-devel libXfixes-devel glib2-devel pam-devel
  ```

- Create build directory under source root directory

  ```
  mkdir build
  ```

- Enter the build directory and generate Makefile through cmake

  ```
  cmake -DCMAKE_INSTALL_PREFIX=/usr ..
  ```

- Compile with make

  ```
  make -j4
  ```

### Installation

- Under the build directory in the source root directory, execute

  ```
  sudo make install
  ```

### Run

- Restart the lightdm service to run lightdm-kiran-greeter

  ```
  sudo systemctl restart lightdm
  ```

- Restart mate-screensaver and lock screen to run kiran-screensaver-dialog 

## Directory structure

source root directory 

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

  To encapsulate the pipes between lib/auth-proxy/auth-pam and kiran-session-guard-checkpass.

- lib/auth-proxy

  Common Certification Related Code Encapsulation 

- lib/common-widgets

  Common Interface Component Code Encapsulation 

- lib/scaling-helper

  Interface scaling code encapsulation 

- libexec/session-guard-checkpass

  PAM authenticated child process in AuthProxy/AuthPam 

- lightdm-greeter

  The implementation of lightdm login manager (lightdm-kiran-greeter).

- screensaver-dialog

  The unlock box of mate-screensaver (kiran-screensaver-dialog) implementation.