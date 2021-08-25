/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kiran-session-guard is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinos.com.cn>
 */

 
#ifndef KIRAN_SCREENSAVER_DIALOG_SRC_AUTH_AUTH_BASE_H_
#define KIRAN_SCREENSAVER_DIALOG_SRC_AUTH_AUTH_BASE_H_

#include <QObject>
#include "auth-define.h"

/**
 * @brief 提供对底层认证相关的一层封装(直接使用PAM模块进行认证或使用lightdm提供的greeter认证接口)，
 * 向上提供给AuthProxy提供统一的接口
 */
class AuthBase : public QObject
{
    Q_OBJECT
public:
    AuthBase(QObject* parent= nullptr){};
    virtual ~AuthBase() = default;

public:
    /**
     * 提供认证模块的相应初始化操作
     * \return
     */
    virtual bool init() = 0;

    /**
     * 开始进行认证
     * \param userName 认证的用户
     * \return 是否开启成功
     */
    virtual bool authenticate(const QString& userName) = 0;

    /**
     * 回复PAM的相应请求
     * \param response
     */
    virtual void respond(const QString& response) = 0;

    /**
     * 是否在认证流程之中
     * \return
     */
    virtual bool inAuthentication() const = 0;

    /**
     * 是否已通过认证
     * \return 是否已通过认证
     */
    virtual bool isAuthenticated() const = 0;

    /**
     * 正在进行认证的用户
     * \return 认证的用户
     */
    virtual QString authenticationUser() const = 0;

    /**
     * 取消正在进行的认证
     */
    virtual void cancelAuthentication() = 0;

signals:
    /**
     * 显示文本提示或错误消息的信号
     * \param text 消息内容
     * \param type 消息类型，错误/文本消息
     */
    void showMessage(QString text,Kiran::MessageType type);

    /**
     * 显示询问消息的信号
     * \param text 消息内容
     * \param type 消息类型,密码类型/明文类型
     */
    void showPrompt(QString text,Kiran::PromptType type);

    /**
     * 认证完成信号
     */
    void authenticationComplete();
};

#endif  //KIRAN_SCREENSAVER_DIALOG_SRC_AUTH_AUTH_BASE_H_
