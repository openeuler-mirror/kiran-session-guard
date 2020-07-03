#include "tabitem.h"
#include "ui_tabitem.h"
#include <QPainter>
#include <QStyleOption>
#include <QStyle>

TabItem::TabItem(const QString &image, const QString &text, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabItem)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->text->setText(text);
    ui->image->setPixmap(image);
}

TabItem::~TabItem()
{
    delete ui;
}

QSize TabItem::sizeHint() const
{
    return QSize(274,60);
}

void TabItem::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);
}
