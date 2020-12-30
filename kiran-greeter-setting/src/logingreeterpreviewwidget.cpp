#include "logingreeterpreviewwidget.h"
#include <QStyleOption>
#include <QPainter>
#include <QDebug>

//#define DEFAULT_BACKGROUND ":/images/default_background.jpg"
#define DEFAULT_BACKGROUND "/usr/share/backgrounds/default.jpg"
#define DEFAULT_SIZE QSize(330,186)

QT_BEGIN_NAMESPACE
Q_WIDGETS_EXPORT void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed = 0);
QT_END_NAMESPACE

LoginGreeterPreviewWidget::LoginGreeterPreviewWidget(QWidget *parent) : QWidget(parent)
{
    setDefaultPreviewBackground();
    setAttribute(Qt::WA_TranslucentBackground);
    this->setFixedSize(DEFAULT_SIZE);
}

LoginGreeterPreviewWidget::~LoginGreeterPreviewWidget()
{

}

void LoginGreeterPreviewWidget::updatePreviewBackground(const QString &path)
{
    m_previewBackground = generatePreviewBackground(path);
    if(m_previewBackground.isNull()){
        setDefaultPreviewBackground();
        return;
    }
    update();
}

void LoginGreeterPreviewWidget::setDefaultPreviewBackground()
{
    m_previewBackground = generatePreviewBackground(DEFAULT_BACKGROUND);
    update();
}

QPixmap LoginGreeterPreviewWidget::generatePreviewBackground(const QString &path)
{
    QPixmap pixmap;
    if( !pixmap.load(path) ){
        qWarning() << "pixmap load" <<path << "failed";
        return QPixmap();
    }
    pixmap = pixmap.scaled(DEFAULT_SIZE,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
    QImage tmp = pixmap.toImage();
    qt_blurImage(tmp,10,true);
    pixmap = QPixmap::fromImage(tmp);
    return pixmap;
}

#define SCALE_FACTOR                 0.171875

#define AVATAR_RADIUS                16
#define AVATAR_EDIT_SPACE            9
#define EDIT_SIZE                    QSize(46,6)

#define TIME_LABEL_SIZE              QSize(36,4)
#define TIME_LABEL_BOTTOM_SPACE      6

#define USER_LIST_ITEM_SIZE          QSize(38,10)
#define USER_LIST_ITEM_SPACING       1
#define USER_LIST_SIZE               QSize(38,53)
#define USER_LIST_BOTTOM_RIGHT_SPACE 6

#define BUTTON_SIZE             QSize(8,6)
#define BUTTON_SPACING          6
#define BUTTON_RIGHT_SPACING    6

void LoginGreeterPreviewWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    opt.initFrom(this);

    ///绘制背景
    painter.setBrush(m_previewBackground);
    painter.drawRoundedRect(QRect(QPoint(0,0),DEFAULT_SIZE),8,8);

    QPainterPath painterPath;

    ///绘制头像框
    QPoint avaterCenter(DEFAULT_SIZE.width()/2,
                        DEFAULT_SIZE.height()/2-AVATAR_RADIUS-AVATAR_EDIT_SPACE);
    painterPath.addEllipse(avaterCenter,AVATAR_RADIUS,AVATAR_RADIUS);

    ///绘制输入框
    painterPath.addRect((DEFAULT_SIZE.width()-EDIT_SIZE.width())/2 ,(DEFAULT_SIZE.height()-EDIT_SIZE.height())/2,
    EDIT_SIZE.width(),EDIT_SIZE.height());

    ///绘制时间框
    painterPath.addRect( (DEFAULT_SIZE.width()-TIME_LABEL_SIZE.width())/2,DEFAULT_SIZE.height()-TIME_LABEL_BOTTOM_SPACE-TIME_LABEL_SIZE.height(),
                         TIME_LABEL_SIZE.width(),TIME_LABEL_SIZE.height());

    ///绘制用户列表
    QRect user5(USER_LIST_BOTTOM_RIGHT_SPACE,DEFAULT_SIZE.height()-USER_LIST_BOTTOM_RIGHT_SPACE-USER_LIST_ITEM_SIZE.height(),
                USER_LIST_ITEM_SIZE.width(),USER_LIST_ITEM_SIZE.height());
    QRect user4(user5.x(),user5.y()-USER_LIST_ITEM_SPACING-USER_LIST_ITEM_SIZE.height(),
                user5.width(),user5.height());
    QRect user3(user5.x(),user4.y()-USER_LIST_ITEM_SPACING-USER_LIST_ITEM_SIZE.height(),
                user5.width(),user5.height());

    painterPath.addRoundedRect(user5,2,2);
    painterPath.addRoundedRect(user4,2,2);
    painterPath.addRoundedRect(user3,2,2);

    ///绘制按钮
    QRect button3( DEFAULT_SIZE.width()-BUTTON_RIGHT_SPACING-BUTTON_SIZE.width(),DEFAULT_SIZE.height()-BUTTON_SIZE.height()-BUTTON_SPACING,
                   BUTTON_SIZE.width(),BUTTON_SIZE.height());
    QRect button2( button3.x()-BUTTON_SPACING-BUTTON_SIZE.width(),button3.y() ,
                   BUTTON_SIZE.width(),BUTTON_SIZE.height());
    QRect button1( button2.x()-BUTTON_SPACING-BUTTON_SIZE.width(),button2.y() ,
                   BUTTON_SIZE.width(),BUTTON_SIZE.height());

    painterPath.addRoundedRect(button1,1,1);
    painterPath.addRoundedRect(button2,1,1);
    painterPath.addRoundedRect(button3,1,1);

    painter.fillPath(painterPath,QColor(255,255,255,80));
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);
}
