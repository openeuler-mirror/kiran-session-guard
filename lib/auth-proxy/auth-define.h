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
#include "guard-global.h"
#include <kiran-authentication-service/kas-authentication-i.h>

GUARD_BEGIN_NAMESPACE
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

enum PromptFromEnum
{
    PROMPT_FROM_PAM,
    PROMPT_FROM_AUTH_SERVICE
};
GUARD_END_NAMESPACE
