//
// Created by lxh on 2020/11/6.
//

#ifndef KIRAN_ACCOUNT_MANAGER_HOVER_TIPS_H
#define KIRAN_ACCOUNT_MANAGER_HOVER_TIPS_H

#include <QWidget>
#include <QMap>

class QLabel;

//TODO:暂时只做了居中显示

class HoverTips : private QWidget{
    Q_OBJECT
public:
    enum HoverTipsTypeEnum{
        HOVE_TIPS_SUC,
        HOVE_TIPS_INFO,
        HOVE_TIPS_WARNING,
        HOVE_TIPS_ERR
    };
    Q_ENUMS(HoverTipsEnum);
public:
    explicit HoverTips(QWidget *parent= nullptr);
    ~HoverTips();

    void setTimeout(quint32 ms);

    void setIcon(HoverTipsTypeEnum typeEnum,const QString &icon);

    void show(HoverTipsTypeEnum typeEnum,const QString &msg);
    void hide();

private:
    void initUI();
    void updatePostion();
    void startHideTimer();
    void stopHideTimer();

protected:
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event);
    void paintEvent(QPaintEvent *event) override;

private:
    QMap<HoverTipsTypeEnum,QString> m_tipsTypeIconMap = {
            {HOVE_TIPS_SUC,":/images/suc.svg"},
            {HOVE_TIPS_INFO,":/images/info.svg"},
            {HOVE_TIPS_WARNING,":/images/warning.svg"},
            {HOVE_TIPS_ERR,":/images/err.svg"}
    };
    QLabel *m_iconLabel;
    QLabel *m_textLabel;
    quint32 m_hideTimeout = 3000;
    int m_hideTimerID = -1;
};


#endif //KIRAN_ACCOUNT_MANAGER_HOVER_TIPS_H
