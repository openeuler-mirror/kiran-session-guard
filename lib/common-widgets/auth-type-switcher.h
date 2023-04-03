/**
 * Copyright (c) 2020 ~ 2023 KylinSec Co., Ltd.
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
#include <kiran-authentication-service/kas-authentication-i.h>
#include <QIcon>
#include <QList>
#include <QSize>
#include <QString>
#include <QWidget>
#include <tuple>
#include "auth-type-switcher-define.h"

namespace Kiran
{
namespace SessionGuard
{
class AuthTypeDrawer;
class AuthTypeSwitcher : public QWidget
{
    Q_OBJECT
public:
    explicit AuthTypeSwitcher(AuthTypeDrawerExpandDirection direction, int radius, QWidget* parent = nullptr);
    ~AuthTypeSwitcher();

    bool getExpaned();

    void setAdjustColorToTheme(bool enable);
    void setAuthTypes(QList<KADAuthType> authtypes);
    int getCurrentAuthType();
    void setCurrentAuthType(int authType);

signals:
    void authTypeChanged(KADAuthType authType);

private slots:
    void onAuthTypeChanged(int authType);

private:
    void clear();

protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;

private:
    bool m_isExpanded = false;
    AuthTypeDrawer* m_drawer;
    // <AuthType,IconPath>
    QMap<KADAuthType, QString> m_authTypeMap;
    int m_radius = 21;
    int m_currentAuthType = -1;
};
}  // namespace SessionGuard
}  // namespace Kiran