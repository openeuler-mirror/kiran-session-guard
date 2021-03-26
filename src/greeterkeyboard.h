#ifndef GREETERKEYBOARD_H
#define GREETERKEYBOARD_H

#include <QObject>
#include <QProcess>

class GreeterKeyboard : public QObject
{
    Q_OBJECT
public:
    static GreeterKeyboard *instance ();
    ~GreeterKeyboard ();

    bool init (QWidget *parent = nullptr);
    void hide ();
    bool isVisible ();
    void showAdjustSize (QWidget *parent = nullptr);
    QWidget *getKeyboard ();
    void keyboardProcessExit ();

public slots:
    void slot_finished (int exitCode, QProcess::ExitStatus exitStatus);

private:
    explicit GreeterKeyboard (QObject *parent = nullptr);

private slots:
    void slotReadyReadStandardOutput ();

private:
    QWidget *m_keyboardWidget;
    QProcess *m_process;
};

#endif // GREETERKEYBOARD_H
