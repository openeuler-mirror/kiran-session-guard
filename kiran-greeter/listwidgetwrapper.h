#ifndef LISTWIDGETWRAPPER_H
#define LISTWIDGETWRAPPER_H

#include <QListWidget>

class ListWidgetWrapper : public QListWidget
{
public:
    ListWidgetWrapper(QWidget* parent=nullptr);
protected:
    virtual QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index,
                                                                 const QEvent *event = Q_NULLPTR) const override;
};

#endif // LISTWIDGETWRAPPER_H
