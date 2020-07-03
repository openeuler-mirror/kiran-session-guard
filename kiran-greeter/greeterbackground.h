#ifndef __GREETER_BACKGROUND_H__
#define __GREETER_BACKGROUND_H__

#include <QWidget>
#include <QPixmap>

class QScreen;
class GreeterBackground : public QWidget
{
    Q_OBJECT
public:
    explicit GreeterBackground(QScreen* screen,QWidget *parent = nullptr);
    ~GreeterBackground();
public:
    ///设置背景窗口绑定的屏幕
    void setScreen(QScreen* screen);
Q_SIGNALS:
    ///鼠标移动到背景窗口传出的信号
    void mouseEnter(GreeterBackground* window);
private slots:
    ///屏幕的大小改变槽函数
    void slotScreenGeometryChanged(const QRect& geometry);
protected:
    ///鼠标移入事件
    virtual void enterEvent(QEvent *event) override;
    ///大小改变事件
    virtual void resizeEvent(QResizeEvent *event) override;
    ///窗口绘制事件
    virtual void paintEvent(QPaintEvent *event) override;
private:
    ///背景窗口绑定的屏幕
    QScreen* m_screen;
    ///绘制的背景图片
    QPixmap m_background;
    ///根据大小拉升之后的背景图片
    QPixmap m_scaledBackground;
};

#endif // __GREETER_BACKGROUND_H__
