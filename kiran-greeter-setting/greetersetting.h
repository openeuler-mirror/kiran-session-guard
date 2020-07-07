#ifndef GREETERSETTING_H
#define GREETERSETTING_H

#include <QWidget>
#include <QGSettings>

namespace Ui {
class GreeterSetting;
}

class GreeterSetting : public QWidget
{
    Q_OBJECT
public:
    explicit GreeterSetting(QWidget *parent = nullptr);
    ~GreeterSetting();
private:
    void initUI();
    void updateFont();
private:
    Ui::GreeterSetting *ui;
    QGSettings m_mateInterfaceSettings;
};

#endif // GREETERSETTING_H
