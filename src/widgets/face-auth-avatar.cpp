#include "face-auth-avatar.h"
#include <QPainter>

FaceAuthAvatar::FaceAuthAvatar(QWidget *parent)
    :QWidget(parent){
    init();
}

FaceAuthAvatar::~FaceAuthAvatar() {

}

void FaceAuthAvatar::startAnimation() {
    m_animation.start();
}

void FaceAuthAvatar::stopAnimation() {
    m_animation.stop();
}

void FaceAuthAvatar::init() {
    m_pixmap.load(":/images/face_auth.png");
    m_scaledPixmap = scalePixmapAdjustSize(m_pixmap);
    m_animation.setTargetObject(this);
    m_animation.setPropertyName("progress");
    m_animation.setStartValue(0);
    m_animation.setEndValue(100);
    m_animation.setDuration(800);
    m_animation.setEasingCurve(QEasingCurve::InSine);
    connect(&m_animation,&QPropertyAnimation::finished,[this](){
        QPropertyAnimation::Direction direction = m_animation.direction()==QAbstractAnimation::Forward?QAbstractAnimation::Backward:QAbstractAnimation::Forward;
        m_animation.setDirection(direction);
        m_animation.start();
    });
}

uint FaceAuthAvatar::progress() {
    return m_progress;
}

void FaceAuthAvatar::setProgress(uint progress) {
    if(m_progress==progress){
        return;
    }
    m_progress = progress;
    update();
}

QPixmap FaceAuthAvatar::scalePixmapAdjustSize(const QPixmap &pixmap) {
    double radius = (this->width()<this->height()?this->width():this->height())/2;
    //NOTE:拉升保持长宽比，尽可能放大，不留白
    QPixmap temp = pixmap.scaled(radius*2,radius*2,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
    return temp;
}

void FaceAuthAvatar::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QPen pen;
    if(!isVisible()){
        return;
    }
    painter.setRenderHints(QPainter::SmoothPixmapTransform|QPainter::HighQualityAntialiasing|QPainter::Antialiasing);
    QRect pixmapRect( (this->width()-m_scaledPixmap.width())/2,
                      (this->height()-m_scaledPixmap.height())/2,
                      m_scaledPixmap.width(),
                      m_scaledPixmap.height());
    painter.drawPixmap(pixmapRect,m_scaledPixmap);
    if( m_animation.state()!=QPropertyAnimation::Running ){
        return;
    }

    int drawY = (pixmapRect.height()/100.0)*m_progress;
    pen.setWidthF(3);
    pen.setColor(QColor("#2eb3ff"));
    painter.setPen(pen);
    painter.drawLine( QLine(pixmapRect.left(),drawY,pixmapRect.right(),drawY));
}

void FaceAuthAvatar::resizeEvent(QResizeEvent *event) {
    if( (!m_pixmap.isNull()) && (!m_scaledPixmap.isNull()) && (m_scaledPixmap.size()!=this->size()) ){
        m_scaledPixmap = scalePixmapAdjustSize(m_pixmap);
    }
    QWidget::resizeEvent(event);
}
