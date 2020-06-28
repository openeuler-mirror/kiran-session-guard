#include "listwidgetwrapper.h"
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>

ListWidgetWrapper::ListWidgetWrapper(QWidget *parent)
    :QListWidget (parent)
{

}

QItemSelectionModel::SelectionFlags ListWidgetWrapper::selectionCommand(const QModelIndex &index, const QEvent *event) const
{
    if( (event!=nullptr) && (event->type()==QEvent::MouseButtonPress) ){
        const QMouseEvent* mouseEvent = (QMouseEvent*) event;
        if( (mouseEvent->modifiers()&Qt::ControlModifier)!=0 ){
            return QItemSelectionModel::NoUpdate;
        }
    }
    return QListWidget::selectionCommand(index,event);
}
