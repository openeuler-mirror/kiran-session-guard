#ifndef TABITEM_H
#define TABITEM_H

#include <QWidget>

namespace Ui
{
    class TabItem;
}

class TabItem : public QWidget
{
    Q_OBJECT
public:
    explicit TabItem (const QString &image, const QString &text, QWidget *parent = nullptr);
    ~TabItem ();
    virtual QSize sizeHint () const;
protected:
    virtual void paintEvent (QPaintEvent *event) Q_DECL_OVERRIDE;
private:
    Ui::TabItem *ui;
};

#endif // TABITEM_H
