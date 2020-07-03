#include "disabledeselectlistwidget.h"
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>

DisableDeselectListWidget::DisableDeselectListWidget(QWidget *parent)
    :QListWidget (parent)
{

}

QItemSelectionModel::SelectionFlags DisableDeselectListWidget::selectionCommand(const QModelIndex &index, const QEvent *event) const
{
    //FIXME:为了避免一些环境下会出现ItemFocus直接设置选中的情况
    if(event==nullptr){
        return QItemSelectionModel::NoUpdate;
    }
    //禁用用户按住鼠标切换用户情况
    if(event->type()==QEvent::MouseMove){
        return QItemSelectionModel::NoUpdate;
    }
    if( (event!=nullptr) && (event->type()==QEvent::MouseButtonPress) ){
        const QMouseEvent* mouseEvent = (QMouseEvent*) event;
        if( (mouseEvent->modifiers()&Qt::ControlModifier)!=0 ){
            return QItemSelectionModel::NoUpdate;
        }
    }
    return QListWidget::selectionCommand(index,event);
}
