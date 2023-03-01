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
#include "auth-define.h"
#include "auth-controller-i.h"
#include "guard-global.h"

/**
 * @brief 提供对底层认证相关的一层封装(例如直接使用PAM认证,使用Greeter接口,使用Polkit接口等底层认证方式)，
 * 向上提供给AuthController提供统一的接口
 */
GUARD_BEGIN_NAMESPACE
class AuthBase
{
public:
    AuthBase(){};
    virtual ~AuthBase() = default;

public:
    /**
     * 提供认证模块的相应初始化操作
     * \return
     */
    virtual bool init(AuthControllerInterface* controllerInterface) = 0;

    virtual bool loginUserSwitchable() { return false; };
    /**
     * 开始进行认证
     * \param userName 认证的用户
     * \return 是否开启成功
     */
    virtual bool authenticate(const QString& userName) = 0;

    /**
     * 回复PAM的相应请求
     * \param response
     */
    virtual void respond(const QString& response) = 0;

    /**
     * 是否在认证流程之中
     * \return
     */
    virtual bool inAuthentication() const = 0;

    /**
     * 是否已通过认证
     * \return 是否已通过认证
     */
    virtual bool isAuthenticated() const = 0;

    /**
     * 正在进行认证的用户
     * \return 认证的用户
     */
    virtual QString authenticationUser() const = 0;

    /**
     * 取消正在进行的认证
     */
    virtual void cancelAuthentication() = 0;
};
GUARD_END_NAMESPACE
