#ifndef LOGINBUTTON_H
#define LOGINBUTTON_H

#include <QColor>
#include <QFont>
#include <QLabel>
#include <QWidget>

namespace Ui
{
class LoginButton;
}

class LoginButton : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool pressed READ pressed WRITE setPressed)
    Q_PROPERTY(QPixmap icon READ icon WRITE setIcon)

public:
    explicit LoginButton(QWidget *parent = nullptr);
    ~LoginButton();
    bool    pressed() const;
    QPixmap icon() const;

public slots:
    void setPressed(bool pressed);
    void setIcon(QPixmap icon);

Q_SIGNALS:
    void sigPressed();
    void sigClicked();

protected:
    virtual void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    virtual void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    Ui::LoginButton *ui;
    bool             m_pressed;
    QPixmap          m_icon;
};

#endif  // LOGINBUTTON_H
