#pragma once
#include <QString>
#include "auth-define.h"

namespace Kiran
{
namespace SessionGuard
{
class AuthControllerInterface
{
    friend class AuthBase;

public:
    virtual void onShowMessage(const QString& text, MessageType type) = 0;
    virtual void onShowPrompt(const QString& text, PromptType type) = 0;
    virtual void onAuthComplete() = 0;
};
}  // namespace SessionGuard
}  // namespace Kiran