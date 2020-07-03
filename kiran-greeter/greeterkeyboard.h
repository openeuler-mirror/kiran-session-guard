#ifndef GREETERKEYBOARD_H
#define GREETERKEYBOARD_H

#include <QObject>
#include <QProcess>
class GreeterKeyboard : public QObject
{
    Q_OBJECT
public:
    static GreeterKeyboard& instance();
    ~GreeterKeyboard() = default;
    void init(QWidget *parent=nullptr);
    bool isVisible();
    void showAdjustSize(QWidget*parent=nullptr);
    void hide();
    QWidget* getKeyboard();
private:
    explicit GreeterKeyboard(QObject *parent = nullptr);
signals:
private slots:
    void slotReadyReadStandardOutput();
private:
    QWidget* m_keyboardWidget;
};

#endif // GREETERKEYBOARD_H
