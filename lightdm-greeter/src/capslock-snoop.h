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

#ifndef CAPSLOCKSNOOP_H
#define CAPSLOCKSNOOP_H

#include <list>
#include <string>

typedef void (*capslock_status_change_callback)(bool isOn, void *user_data);
typedef struct _CapsLockSnoopPrivate CapsLockSnoopPrivate;

/**
 * @brief CapsLock简单全局监控
 */
class CapsLockSnoop
{
public:
    CapsLockSnoop();
    ~CapsLockSnoop();
    bool start(capslock_status_change_callback callback, void *user_data, std::string &error);
    bool stop();

private:
    bool         getCapsLockCurrentState(bool &isOn, std::string &error);
    static void *thread_record_func(void *param);

public:
    //NOTE:只是为了在回调record_intercept_proc_callback中能访问到
    CapsLockSnoopPrivate *m_private;
};

#endif  // CAPSLOCKSNOOP_H
