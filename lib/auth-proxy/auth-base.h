 /**
  * @Copyright (C) 2020 ~ 2021 KylinSec Co., Ltd.
  *
  * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
  *
  * This program is free software; you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation; either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program; If not, see <http: //www.gnu.org/licenses/>. 
  */
 
#ifndef KIRAN_SCREENSAVER_DIALOG_SRC_AUTH_AUTH_BASE_H_
#define KIRAN_SCREENSAVER_DIALOG_SRC_AUTH_AUTH_BASE_H_

#include <QObject>
#include "auth-define.h"

/**
 * @brief 提供对底层认证相关的一层封装(直接使用PAM模块进行认证或使用lightdm提供的greeter认证接口)，
 * 向上提供统一的接口
 */
class AuthBase : public QObject
{
    Q_OBJECT
public:
    AuthBase(QObject* parent= nullptr){};
    virtual ~AuthBase() = default;

public:
    virtual bool init() = 0;
    virtual bool authenticate(const QString& userName) = 0;
    virtual void respond(const QString& response) = 0;
    virtual bool inAuthentication() const = 0;
    virtual bool isAuthenticated() const = 0;
    virtual QString authenticationUser() const = 0;
    virtual void cancelAuthentication() = 0;

signals:
    void showMessage(QString text,Kiran::MessageType type);
    void showPrompt(QString text,Kiran::PromptType type);
    void authenticationComplete();
};

#endif  //KIRAN_SCREENSAVER_DIALOG_SRC_AUTH_AUTH_BASE_H_
