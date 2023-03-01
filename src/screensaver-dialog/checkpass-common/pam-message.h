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
#pragma once
#include <QJsonDocument>
#include <QString>
#include <QtCore>
#include "guard-global.h"

GUARD_LOCKER_BEGIN_NAMESPACE

class PamEvent
{
    Q_GADGET
public:
    enum Type
    {
        Error,
        PromptRequest,
        PromptReply,
        Message,
        Complete,
        LAST
    };
    Q_ENUM(Type);

    PamEvent(Type type, QString textInfo):m_type(type),m_text(textInfo){};
    virtual ~PamEvent(){};

    inline Type type() { return m_type; };
    inline QString text() { return m_text; };

private:
    Type m_type;
    QString m_text;
};

class ErrorEvent : public PamEvent
{
public:
    ErrorEvent(QString textInfo) : PamEvent(Error, textInfo){};
    ~ErrorEvent()=default;
};

class PromptRequestEvent : public PamEvent
{
public:
    PromptRequestEvent(bool isSecret, QString textInfo)
        : PamEvent(PromptRequest, textInfo),
          m_isSecret(isSecret){};
    ~PromptRequestEvent() = default;

    inline bool secret() { return m_isSecret; };

private:
    bool m_isSecret;
};

class PromptReplyEvent : public PamEvent
{
public:
    PromptReplyEvent(bool result, QString textInfo)
        : PamEvent(PromptReply, textInfo)
          ,m_result(result){};
    ~PromptReplyEvent() = default;

    inline bool result() { return m_result; };

private:
    bool m_result;
};

class MessageEvent : public PamEvent
{
public:
    MessageEvent(bool isError, QString textInfo)
        : PamEvent(Message, textInfo)
          ,m_isError(isError){};
    ~MessageEvent() = default;

    inline bool isError() { return m_isError; };

private:
    bool m_isError;
};

class CompleteEvent : public PamEvent
{
public:
    CompleteEvent(bool isComplete, bool authRes, QString textInfo)
        : PamEvent(Complete, textInfo),
          m_isComplete(isComplete),
          m_authResult(authRes){};
    ~CompleteEvent() = default;

    inline bool isComplete() { return m_isComplete; };
    inline bool authResult() { return m_authResult; };

private:
    bool m_isComplete;
    bool m_authResult;
};

bool kiran_pam_message_send_event(int fd, PamEvent* event);
bool kiran_pam_message_recv_event(int fd, PamEvent** event);
void kiran_pam_message_free(PamEvent** event);

GUARD_LOCKER_END_NAMESPACE