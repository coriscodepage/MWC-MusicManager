#ifndef PRIMARYLISTDELEGATE_H
#define PRIMARYLISTDELEGATE_H

#include <QStyledItemDelegate>

class PrimaryListDelegate : public QStyledItemDelegate
{
public:
    PrimaryListDelegate();
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // PRIMARYLISTDELEGATE_H
