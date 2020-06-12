#ifndef GREETERMENUITEM_H
#define GREETERMENUITEM_H

#include <QWidget>

class QButtonGroup;
class QLabel;
class QCheckBox;
class QEvent;
class GreeterMenuItem : public QWidget
{
    Q_OBJECT
public:
    explicit GreeterMenuItem(const QString&text,bool checkable,QWidget*parent = nullptr);
    void setExclusiveGroup(QButtonGroup *group);
    QString getActionName();
    void setChecked(bool isChecked);
signals:
    void sigChecked(QString action);
public slots:
private:
    void initUI();
protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
private:
    bool m_checkable;
    QString m_actionName;
    QLabel* m_label;
    QCheckBox* m_checkbox;
};

#endif // GREETERMENUITEM_H
