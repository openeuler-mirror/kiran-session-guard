/**
 * Copyright (c) 2020 ~ 2024 KylinSec Co., Ltd.
 * kiran-session-guard is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 *
 * Author:     liuxinhao <liuxinhao@kylinsec.com.cn>
 */
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