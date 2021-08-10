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
