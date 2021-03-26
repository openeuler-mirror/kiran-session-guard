#ifndef LOGINGREETERPREVIEWWIDGET_H
#define LOGINGREETERPREVIEWWIDGET_H

#include <QWidget>

class LoginGreeterPreviewWidget : public QWidget
{
Q_OBJECT
public:
    explicit LoginGreeterPreviewWidget (QWidget *parent = nullptr);
    ~LoginGreeterPreviewWidget ();
    void updatePreviewBackground (const QString &path);
private:
    void setDefaultPreviewBackground ();
    QPixmap generatePreviewBackground (const QString &path);
signals:
protected:
    virtual void paintEvent (QPaintEvent *event) Q_DECL_OVERRIDE;
private:
    QPixmap m_previewBackground;
};

#endif // LOGINGREETERPREVIEWWIDGET_H
