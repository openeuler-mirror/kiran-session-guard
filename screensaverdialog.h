#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QAbstractNativeEventFilter>
#include <QPropertyAnimation>
#include "pamauthproxy.h"

namespace Ui {
class ScreenSaverDialog;
}

class QMenu;
class ScreenSaverDialog : public QWidget,PamAuthCallback
{
    Q_OBJECT
public:
    explicit ScreenSaverDialog(QWidget *parent = nullptr);
    virtual ~ScreenSaverDialog();
    void setSwitchUserEnabled(bool enable);
private:
    void InitUI();
    QString getUser();
    Q_INVOKABLE void startUpdateTimeTimer();
    Q_INVOKABLE void updateTimeLabel();
    QString getCurrentDateTime();
private:
    virtual void requestResponse(const char* msg,bool visiable) Q_DECL_OVERRIDE;
    Q_INVOKABLE void requestResponse(const QString& msg,bool visiable);
    virtual void onDisplayError(const char* msg) Q_DECL_OVERRIDE;
    virtual void onDisplayTextInfo(const char* msg) Q_DECL_OVERRIDE;
private:
    ///通过标准输出回复ScreenSaver接口
    void printWindowID();
    void responseOkAndQuit();
    Q_INVOKABLE void responseCancelAndQuit();
    void responseNoticeAuthFailed();
private slots:
    void slotAuthenticateComplete(bool isSuccess);
protected:
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
    virtual void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
private:
    Ui::ScreenSaverDialog *ui;
    PamAuthProxy m_authProxy;
    QString m_userName;
    QPixmap m_background;
    QPixmap m_scaledBackground;
    QMenu* m_powerMenu;
};

#endif // WIDGET_H
