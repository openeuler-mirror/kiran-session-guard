#ifndef GREETERSETTING_H
#define GREETERSETTING_H

#include <QWidget>

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
private:
    Ui::GreeterSetting *ui;
};

#endif // GREETERSETTING_H
