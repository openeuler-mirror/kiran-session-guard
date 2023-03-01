#pragma once
#include <QObject>
#include <PolkitQt1/Agent/Listener>
#include <PolkitQt1/Details>
#include <QString>
#include <kiran-titlebar-window.h>
#include "auth-base.h"

namespace Ui
{
class Dialog;
}

class KiranPasswdEdit;

QT_BEGIN_NAMESPACE
class QGridLayout;
class QLabel;
class QComboBox;
QT_END_NAMESPACE

GUARD_BEGIN_NAMESPACE
class AuthController;
class AuthTypeSwitcher;
GUARD_END_NAMESPACE

GUARD_POLKIT_AGENT_BEGIN_NAMESPACE
class AuthPolkit;
class AuthInfo
{
public:
    QString actionID;
    QString message;
    QString iconName;
    PolkitQt1::Details details;
    QString cookie;
    PolkitQt1::Identity::List identities;
    PolkitQt1::Agent::AsyncResult* result;
    void dump();
};

class Dialog : public KiranTitlebarWindow
{
    Q_OBJECT
public:
    explicit Dialog(QWidget* parent = nullptr);
    ~Dialog();

public:
    bool init(const AuthInfo& authInfo);

signals:
    void cancelled();
    void completed(bool isSuccess);

private:
    void initUI();
    bool setAuthInfo(const AuthInfo& authInfo);
    void startAuth(const QString& userName);

private slots:
    void onCancelClicked();
    void onOkClicked();
    void onCurrentUserChanged(int idx);
    void onCurrentAuthTypeChanged(KADAuthType authType);

    void onNotifyAuthMode(KADAuthMode mode);
    void onSupportedAuthTypeChanged(QList<KADAuthType> authTypes);
    void onNotifyAuthTypeChanged(KADAuthType authType);

    void onAuthComplete(bool success);
    void onAuthShowPrompt(const QString& text, PromptType promptType);
    void onAuthShowMessage(const QString& text, MessageType msgType);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    Ui::Dialog* ui;
    AuthController* m_authController;
    AuthTypeSwitcher* m_switcher;
    bool m_havePrompt = false;
    int m_triesCount = 0;
};
GUARD_POLKIT_AGENT_END_NAMESPACE