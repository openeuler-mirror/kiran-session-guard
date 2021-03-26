#ifndef __GREETER_SCREEN_MANAGER_H__
#define __GREETER_SCREEN_MANAGER_H__

#include <QObject>
#include <QMap>
#include <QScreen>

class QScreen;
class GreeterLoginWindow;
class GreeterBackground;

class GreeterScreenManager : public QObject
{
    Q_OBJECT
public:
    GreeterScreenManager (QObject *parent = nullptr);
    ~GreeterScreenManager ();
    void init ();
private slots:
    void slotScreenAdded (QScreen *screen);
    void slotScreenRemoved (QScreen *screen);
    void mouseEnterInWindow (GreeterBackground *background);
private:
    void newScreenBackgroundWidget (QScreen *screen);
    void setGreeterOnBackground (GreeterBackground *background);
private:
    //保存屏幕和屏幕的映射
    QMap<QScreen *, GreeterBackground *> m_BackgroundWidgetMap;
    //登录内容窗口
    GreeterLoginWindow *m_greeterWindow;
};

#endif // __GREETER_SCREEN_MANAGER_H__
