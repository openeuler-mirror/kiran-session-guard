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
    if( (event!=nullptr) && (event->type()==QEvent::MouseButtonPress) ){
        const QMouseEvent* mouseEvent = (QMouseEvent*) event;
        if( (mouseEvent->modifiers()&Qt::ControlModifier)!=0 ){
            return QItemSelectionModel::NoUpdate;
        }
    }
    return QListWidget::selectionCommand(index,event);
}
