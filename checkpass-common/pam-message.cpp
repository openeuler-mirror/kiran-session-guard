/**
 * Copyright (c) 2020 ~ 2021 KylinSec Co., Ltd.
 * kinar-session-guard is licensed under Mulan PSL v2.
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


#include "pam-message.h"

#include <QDataStream>
#include <QJsonDocument>

#include <qt5-log-i.h>
#include <unistd.h>
#include <QJsonObject>

bool kiran_pam_message_send(int fd, QJsonDocument& content);
bool kiran_pam_message_recv(int fd, QJsonDocument& content);

bool kiran_pam_message_is_valid(const QJsonDocument& content)
{
    return !content.isNull() && content.isObject();
}

bool kiran_pam_message_send(int fd, QJsonDocument& content)
{
    QByteArray byteArray;
    QDataStream sendStream(&byteArray, QIODevice::ReadWrite);

    QByteArray qByteArray = content.toJson();
    uint32_t contentLength = qByteArray.length();

    if (sendStream.writeRawData((const char*)&contentLength, sizeof(contentLength)) == -1)
    {
        return false;
    }

    if (sendStream.writeRawData((const char*)qByteArray.data(), qByteArray.length()) == -1)
    {
        return false;
    }

    if (write(fd, byteArray.data(), byteArray.length()) != byteArray.length())
    {
        return false;
    }

    KLOG_DEBUG() << "send json:" << content;
    return true;
}

bool kiran_pam_message_recv(int fd, QJsonDocument& content)
{
    uint32_t length = 0;

    if (read(fd, &length, sizeof(length)) < 0)
    {
        KLOG_DEBUG() << "read error:" << strerror(errno);
        return false;
    }

    QByteArray byteArray;
    byteArray.resize(length);
    if (read(fd, byteArray.data(), length) < length)
    {
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(byteArray);
    if (doc.isNull())
    {
        return false;
    }

    content = doc;
    KLOG_DEBUG() << "recv json:" << content;
    return true;
}

bool kiran_pam_message_send_event(int fd, PamEvent* event)
{
    PamEvent::Type type = event->type();
    if (type < PamEvent::Error || type >= PamEvent::LAST)
    {
        KLOG_ERROR() << "event type isvalid," << type;
        return false;
    }

    QJsonDocument jsonDoc;
    QJsonObject jsonObject;

    jsonObject["event"] = type;
    jsonObject["text"] = event->text();
    switch (type)
    {
    case PamEvent::Error:
        break;
    case PamEvent::PromptRequest:
        jsonObject["secret"] = dynamic_cast<PromptRequestEvent*>(event)->secret();
        break;
    case PamEvent::PromptReply:
        jsonObject["reply_result"] = dynamic_cast<PromptReplyEvent*>(event)->result();
        break;
    case PamEvent::Message:
        jsonObject["error_info"] = dynamic_cast<MessageEvent*>(event)->isError();
        break;
    case PamEvent::Complete:
        jsonObject["complete"] = dynamic_cast<CompleteEvent*>(event)->isComplete();
        jsonObject["auth_result"] = dynamic_cast<CompleteEvent*>(event)->authResult();
        break;
    default:
        KLOG_ERROR() << "not supported this event type:" << type;
        return false;
    }

    jsonDoc.setObject(jsonObject);
    kiran_pam_message_send(fd, jsonDoc);

    return true;
}

bool kiran_pam_message_recv_event(int fd, PamEvent** event)
{
    QJsonDocument jsonDoc;

    if (!kiran_pam_message_recv(fd, jsonDoc))
    {
        return false;
    }

    QJsonObject jsonObject = jsonDoc.object();
    if (jsonObject.isEmpty() || jsonObject["event"].isNull())
    {
        KLOG_ERROR() << "invalid json format!";
        return false;
    }

    PamEvent::Type eventType = static_cast<PamEvent::Type>(jsonObject["event"].toInt());
    if (eventType < PamEvent::Error || eventType >= PamEvent::LAST || jsonObject["text"].isNull())
    {
        KLOG_ERROR() << "not supported this event type:" << eventType;
        return false;
    }

    QString textInfo = jsonObject["text"].toString();
    switch (eventType)
    {
    case PamEvent::Error:
    {
        auto errorEvent = new ErrorEvent(textInfo);
        *event = errorEvent;
        return true;
    }
    case PamEvent::PromptRequest:
    {
        if (jsonObject["secret"].isNull() || !jsonObject["secret"].isBool())
        {
            KLOG_ERROR() << "invalid prompt req format";
            return false;
        }
        bool isSecret = jsonObject["secret"].toBool();
        auto promptReqEvent = new PromptRequestEvent(isSecret, textInfo);
        *event = promptReqEvent;
        return true;
    }
    case PamEvent::PromptReply:
    {
        if (jsonObject["reply_result"].isNull() || !jsonObject["reply_result"].toBool())
        {
            KLOG_ERROR() << "invalid reply format";
            return false;
        }
        bool result = jsonObject["reply_result"].toBool();
        auto promptRep = new PromptReplyEvent(result, textInfo);
        *event = promptRep;
        return true;
    }
    case PamEvent::Message:
    {
        if (jsonObject["error_info"].isNull() || !jsonObject["error_info"].isBool())
        {
            KLOG_ERROR() << "invalid message format";
            return false;
        }
        bool isError = jsonObject["error_info"].toBool();
        auto msgEvent = new MessageEvent(isError, textInfo);
        *event = msgEvent;
        return true;
    }
    case PamEvent::Complete:
    {
        if (jsonObject["complete"].isNull() || jsonObject["auth_result"].isNull() ||
            !jsonObject["complete"].isBool() || !jsonObject["auth_result"].isBool())
        {
            KLOG_ERROR() << "invalid complete format";
            return false;
        }
        bool authRes = jsonObject["auth_result"].toBool();
        bool authComplete = jsonObject["complete"].toBool();
        auto authCompleteEvent = new CompleteEvent(authComplete, authRes, textInfo);
        *event = authCompleteEvent;
        return true;
    }
    default:
        KLOG_ERROR() << "not supported this event type:" << eventType;
        return false;
    }

    return false;
}

void kiran_pam_message_free(PamEvent** event)
{
    delete *event;
    *event = nullptr;
}
