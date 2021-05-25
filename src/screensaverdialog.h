#ifndef WIDGET_H
#define WIDGET_H

#include <QAbstractNativeEventFilter>
#include <QPropertyAnimation>
#include <QWidget>

#include "pamauthproxy.h"

namespace Ui
{
class ScreenSaverDialog;
}

class QMenu;

class ScreenSaverDialog : public QWidget
{
    Q_OBJECT
public:
    enum AuthType
    {
        AUTH_TYPE_PASSWD,
        AUTH_TYPE_FINGER,
        AUTH_TYPE_FACE
    };
    explicit ScreenSaverDialog(QWidget *parent = nullptr);
    virtual ~ScreenSaverDialog();
    void setSwitchUserEnabled(bool enable);

private:
    void init();
    void initPamAuthProxy();
    void initUI();

    QString getUser();
    Q_INVOKABLE void startUpdateTimeTimer();
    Q_INVOKABLE void updateTimeLabel();
    QString getCurrentDateTime();
    void updateCurrentAuthType(ScreenSaverDialog::AuthType type);

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
    void slotShowMessage(QString text, PamAuthProxy::MessageType type);
    void slotShowPrompt(QString text, PamAuthProxy::PromptType type);
    void slotAuthenticationComplete();

protected:
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    virtual void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    virtual void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    Ui::ScreenSaverDialog *ui;
    PamAuthProxy m_authProxy;
    QString m_userName;
    QPixmap m_background;
    QPixmap m_scaledBackground;
    QMenu *m_powerMenu;
    AuthType m_authType = AUTH_TYPE_PASSWD;
    bool m_havePrompt = false;
    bool m_haveErr = false;
};

#endif  // WIDGET_H
