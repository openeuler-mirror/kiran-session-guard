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


#ifndef KIRAN_SCREENSAVER_DIALOG_SRC_AUTH_AUTH_MSG_QUEUE_BASE_H_
#define KIRAN_SCREENSAVER_DIALOG_SRC_AUTH_AUTH_MSG_QUEUE_BASE_H_

#include <QObject>
#include "auth-define.h"

class Q_DECL_IMPORT AuthMsgQueueBase : public QObject
{
    Q_OBJECT
public:
    AuthMsgQueueBase(QObject* parent = nullptr) : QObject(parent){};
    virtual ~AuthMsgQueueBase() = default;

    ///开始事件分发
    virtual void startDispatcher() = 0;

    ///停止事件分发并清空消息队列
    virtual void stopDispatcher() = 0;

    ///消息间隔
    virtual void setInterval(int seconds) = 0;
    virtual int interval() = 0;

    ///AuthProxy通过以下接口添加认证相关的消息
    virtual void append(Kiran::PromptFromEnum promptFrom, Kiran::PromptType promptType, const QString& msg) = 0;
    virtual void append(Kiran::MessageType messageType, const QString& msg) = 0;
    virtual void appendAuthCompleteMsg(bool authRes) = 0;

signals:
    ///通知AuthProxy认证事件
    void showMessage(QString text, Kiran::MessageType type);
    void showPrompt(QString text, Kiran::PromptType type,Kiran::PromptFromEnum promptFrom);
    void authenticationComplete(bool authRes);
};

#endif  //KIRAN_SCREENSAVER_DIALOG_SRC_AUTH_AUTH_MSG_QUEUE_BASE_H_
