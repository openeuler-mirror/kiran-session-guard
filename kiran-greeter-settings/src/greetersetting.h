#ifndef GREETERSETTING_H
#define GREETERSETTING_H

#include <QWidget>
#include <QGSettings>
#include <kiranwidgets-qt5/kiran-titlebar-window.h>

namespace Ui
{
    class GreeterSetting;
}

class KiranSwitchButton;
class GreeterSetting : public KiranTitlebarWindow
{
    Q_OBJECT
public:
    explicit GreeterSetting ();
    ~GreeterSetting ();
private:
    void initUI ();
    bool initUserComboBox ();
    void updateFont ();
private:
    Ui::GreeterSetting *ui;
    QGSettings m_mateInterfaceSettings;
    KiranSwitchButton* m_hideUserListBtn;
    KiranSwitchButton* m_enableManualLoginBtn;
};

#endif // GREETERSETTING_H
