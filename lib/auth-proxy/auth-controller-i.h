#pragma once
#include <QString>
#include "auth-define.h"
#include "guard-global.h"

GUARD_BEGIN_NAMESPACE
class AuthControllerInterface
{
    friend class AuthBase;
public:
    virtual void onShowMessage(const QString& text, MessageType type) = 0;
    virtual void onShowPrompt(const QString& text, PromptType type) = 0;
    virtual void onAuthComplete() = 0;
};
GUARD_END_NAMESPACE