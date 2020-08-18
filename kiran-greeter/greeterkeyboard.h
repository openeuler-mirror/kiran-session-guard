#ifndef GREETERKEYBOARD_H
#define GREETERKEYBOARD_H

#include <QObject>
#include <QProcess>

class GreeterKeyboard : public QObject
{
    Q_OBJECT
public:
    static GreeterKeyboard* instance();
    ~GreeterKeyboard() = default;
    void init(QWidget *parent=nullptr);
    bool isVisible();
    void showAdjustSize(QWidget*parent=nullptr);
    void hide();
    QWidget* getKeyboard();
    void resetParentAndTermProcess();
public slots:
    void slot_finished(int exitCode,QProcess::ExitStatus exitStatus);
private:
    explicit GreeterKeyboard(QObject *parent = nullptr);
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
signals:
private slots:
    void slotReadyReadStandardOutput();
private:
    QWidget* m_keyboardWidget;
    QProcess* m_process;
};

#endif // GREETERKEYBOARD_H
