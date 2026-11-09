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
#include <QLabel>
#include <QWidget>
#include "auth-define.h"

namespace Ui
{
class LoginFrame;
}

QT_BEGIN_NAMESPACE
class QLayoutItem;
class QProcess;
class QTimer;
class QToolButton;
QT_END_NAMESPACE

// TODO:对于AuthController的修改未改完
namespace Kiran
{
namespace SessionGuard
{
// 提供基础的认证界面给登录以及解锁框
// 负责基础的认证流程
class AuthController;
class UserListWidget;
class AuthBase;
class AuthTypeSwitcher;
class FaceDaemonSignalListener;
class LoginFrame : public QWidget
{
    Q_OBJECT
protected:
    enum ControlPageEnum
    {
        CONTROL_PAGE_PROMPT_EDIT,  // 输入框模式
        CONTROL_PAGE_REAUTH,       // 重新认证,认证失败且过程未存在Prompt消息,则不应自动重新开始认证,而是应该显示重新认证按钮,等待用户手动触发认证
        CONTROL_PAGE_EMPTY         // 控制界面为空,用于进行不需要用户操作的生物认证
    };
    enum EditMode
    {
        EDIT_MODE_USER_NAME,      // 输入框输入用户名
        EDIT_MODE_PROMPT_RESPOSE  // 输入框输入PAM询问的prompt回复
    };

public:
    LoginFrame(QWidget* parent = nullptr);
    ~LoginFrame();

    void initAuth(AuthBase* auth);
    void reset();

    void startAuthUser(const QString& userName);
    void setAuthUserInfo(const QString& userName);
    void setSpecifyUser(const QString& user) { m_specifyUser = user; }
    void setTips(MessageType type, const QString& text);
    void setAutoAuthCodeRequest(bool autoMode) { m_autoAuthCodeRequest = autoMode; }

    void setLeftTopWidget(QWidget* w);
    void setLeftBottomWidget(QWidget* w);
    void setRightTopWidget(QWidget* w);
    void setRightBottomWidget(QWidget* w);
    void addWidgetToCenterBottomWidget(int index, QWidget* w, Qt::Alignment align = Qt::AlignCenter);
    void addLayoutItemToCenterBottomWidget(int index, QLayoutItem* item);
    int appendControlPage(QWidget* controlWidget);
    void switchControlPage(int pageIdx);

protected:
    void setEditFocus(int delayMs = 0);
    virtual void onAuthTypeChanged(KADAuthType type);
    virtual void onSupportedAuthTypeChanged(QList<KADAuthType> supportedTypes);
    /**
     * @brief 用户点击认证类型圆按钮（展开/收起）；锁屏被动循环可在此结束
     */
    virtual void onAuthTypeSwitcherActivated() {}

    /**
     * @brief 触发重新认证按钮点击（供子类被动认证循环使用）
     * @note 利用 QAbstractButton::click() 内置 isEnabled() 守卫——disabled 时为空操作
     */
    void triggerReAuthButton();

    /**
     * @brief 底层认证是否进行中（供子类被动循环跳过叠触发）
     */
    bool isAuthInProgress() const;

    /**
     * @brief 获取当前认证类型（供子类判断是否启动被动循环）
     */
    KADAuthType lastAuthType() const { return m_lastAuthType; }

private:
    void initUI();
    void respond(const QString& response);

    virtual QString getCurrentDateTime();
    Q_INVOKABLE void startUpdateTimeTimer();
    Q_INVOKABLE void updateTimeLabel();

    void capsLockStatusChanged(bool on);
    // 需认证用的户名输入完成,子类对认证用户名进行判断,是否调用开始认证
    virtual void authUserInputed(const QString& userName) = 0;
    // 认证完成,子类根据认证结果以及用户名执行各自操作
    virtual void authenticateComplete(bool authRes, const QString& userName) = 0;
    // 认证头像下方是否显示FullName
    virtual bool shouldShowFullName() = 0;

private slots:
    void onShowMessage(const QString& text, MessageType type);
    void onShowPrmpt(const QString& text, PromptType type);
    void onAuthComplete(bool authRes);
    void onNotifyAuthMode(KADAuthMode mode);
    void onAuthUserPropertyChanged();
    void onFaceLeaveDetected(QString json);

private:
    // 按认证类型显示控制页面
    void updateControlPageForAuthType(KADAuthType authType);
    // 判断认证类型是否需要输入框
    bool isEmptyControlAuthType(KADAuthType type) const;
    void updateFacePreviewVisibility();
    bool isFaceAuthType(KADAuthType type) const;
    bool isSoftCodeAuthType(KADAuthType type) const;
    bool canStartNewAuth() const;
    void enableReAuthButton();
    /**
     * @brief 申请任务结束后的统一收尾
     * @param process 本次申请进程，不可为空
     * @note 幂等：同一 process 只收尾一次；仅清空进程指针，不再开冷却（冷却在点击时已启动）
     */
    void completeAuthCodeRequest(QProcess* process);
    /**
     * @brief 点击申请后立即启动 3 秒冷却
     * @note 冷却只负责按钮禁用/恢复；与申请进程是否结束无关
     */
    void startAuthCodeRequestCooldown();
    /**
     * @brief 冷却定时器到点后恢复申请授权码按钮使能
     */
    void onAuthCodeRequestCooldownTimeout();
    /**
     * @brief 终止当前申请进程并递增 epoch（不作 waitForFinished）
     * @note 用于切类型/reset，或冷却结束后再次点击时替换旧进程
     */
    void terminateAuthCodeRequestProcess();
    /**
     * @brief 复位申请授权码按钮的冷却与进程状态
     * @return 是否终止了仍在运行的申请进程
     * @note 停止冷却；终止残留进程并作废旧回调；恢复按钮使能
     */
    bool resetAuthCodeRequestButtonState();

    Ui::LoginFrame* ui;
    QWidget* m_leftTopWidget = nullptr;
    QWidget* m_leftBottomWidget = nullptr;
    QWidget* m_rightTopWidget = nullptr;
    QWidget* m_rightBottomWidget = nullptr;
    AuthController* m_authController = nullptr;
    EditMode m_editMode;
    bool m_prompted = false;
    QString m_specifyUser;
    AuthTypeSwitcher* m_switcher = nullptr;
    FaceDaemonSignalListener* m_faceDaemonSignals = nullptr;
    KADAuthType m_lastAuthType = KAD_AUTH_TYPE_NONE;
    bool m_facePreviewSuppressedByLeave = false;
    bool m_inUiReset = false;
    void updateRequestAuthCodeButtonVisibility(KADAuthType type);
    QToolButton* m_btnRequestAuthCodeButton = nullptr;
    QTimer* m_authCodeRequestCooldownTimer = nullptr;  // 申请授权码防连点冷却定时器
    QProcess* m_authCodeRequestProcess = nullptr;      // 正在运行的申请授权码进程
    quint64 m_authCodeRequestEpoch = 0;                // 申请轮次，作废后丢弃旧回调
    bool m_authCodeRequestButtonWanted = false;        // 上次是否应显示申请按钮（不用 isVisible）
    QList<KADAuthType> m_supportedAuthTypes;
    bool m_autoAuthCodeRequest = false;
};
}  // namespace SessionGuard
}  // namespace Kiran
