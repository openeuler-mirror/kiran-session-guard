//
// Created by lxh on 2021/7/27.
//

#ifndef KIRAN_SCREENSAVER_DIALOG_SRC_AUTH_AUTH_DEFINE_H_
#define KIRAN_SCREENSAVER_DIALOG_SRC_AUTH_AUTH_DEFINE_H_

namespace Kiran
{
enum PromptType
{
    PromptTypeQuestion,
    PromptTypeSecret
};

enum MessageType
{
    MessageTypeInfo,
    MessageTypeError
};

enum AuthType
{
    AUTH_TYPE_PASSWD = (1 << 0),
    AUTH_TYPE_FINGER = (1 << 1),
    AUTH_TYPE_FACE = (1 << 2)
};
inline AuthType operator|(AuthType type1,AuthType type2)
{
    return static_cast<AuthType>(static_cast<int>(type1)|static_cast<int>(type2));
}

enum PromptFromEnum
{
    PROMPT_FROM_PAM,
    PROMPT_FROM_AUTH_SERVICE
};
}  // namespace Kiran
#endif  //KIRAN_SCREENSAVER_DIALOG_SRC_AUTH_AUTH_DEFINE_H_
