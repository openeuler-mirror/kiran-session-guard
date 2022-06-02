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
