#pragma once
#include "auth-type-switcher-define.h"
#include <kiran-authentication-service/kas-authentication-i.h>
#include <QIcon>
#include <QList>
#include <QSize>
#include <QString>
#include <QWidget>
#include <tuple>

//TODO:图标随深浅色变化
GUARD_BEGIN_NAMESPACE
class AuthTypeDrawer;
class AuthTypeSwitcher : public QWidget
{
    Q_OBJECT
public:
    explicit AuthTypeSwitcher(AuthTypeDrawerExpandDirection direction,int radius,QWidget* parent = nullptr);
    ~AuthTypeSwitcher();

    bool getExpaned();

    void setAuthTypes(QList<KADAuthType> authtypes);
    int  getCurrentAuthType();
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
GUARD_END_NAMESPACE