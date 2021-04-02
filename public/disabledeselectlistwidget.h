#ifndef LISTWIDGETWRAPPER_H
#define LISTWIDGETWRAPPER_H

#include <QItemSelectionModel>
#include <QListWidget>

class DisableDeselectListWidget : public QListWidget
{
public:
    DisableDeselectListWidget(QWidget *parent = nullptr);

protected:
    virtual QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index,
                                                                 const QEvent *     event = Q_NULLPTR) const override;
};

#endif  // LISTWIDGETWRAPPER_H
