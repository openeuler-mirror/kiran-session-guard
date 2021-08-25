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

#ifndef WIDGET_H
#define WIDGET_H

#include <QAbstractNativeEventFilter>
#include <QPropertyAnimation>
#include <QWidget>
#include "auth-base.h"

namespace Ui
{
class ScreenSaverDialog;
}

class QMenu;
class AuthProxy;
class ScreenSaverDialog : public QWidget
{
    Q_OBJECT
public:
    explicit ScreenSaverDialog(QWidget *parent = nullptr);
    virtual ~ScreenSaverDialog();
    void setSwitchUserEnabled(bool enable);

private:
    void init();
    void initAuth();
    void initUI();

    Q_INVOKABLE void startUpdateTimeTimer();
    Q_INVOKABLE void updateTimeLabel();
    QString getCurrentDateTime();
    void updateCurrentAuthType(Kiran::AuthType type);

private:
    ///开始进行PAM认证
    void startAuth();

    ///通过标准输出回复ScreenSaver接口
    void printWindowID();
    void responseOkAndQuit();
    Q_INVOKABLE void responseCancelAndQuit();
    void responseNoticeAuthFailed();

    ///切换输入框到重新认证按钮
    void switchToReauthentication();

    ///显示输入框
    void switchToPromptEdit();

private slots:
    void slotShowMessage(QString text, Kiran::MessageType messageType);
    void slotShowPrompt(QString text, Kiran::PromptType promptType);
    void slotAuthenticationComplete(bool authRes);

protected:
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    virtual void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    virtual void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    Ui::ScreenSaverDialog *ui;
    AuthProxy* m_authProxy = nullptr;
    QString m_userName;
    QPixmap m_background;
    QPixmap m_scaledBackground;
    QMenu *m_powerMenu = nullptr;
    Kiran::AuthType m_authType = Kiran::AUTH_TYPE_PASSWD;
    bool m_havePrompt = false;
};

#endif  // WIDGET_H
