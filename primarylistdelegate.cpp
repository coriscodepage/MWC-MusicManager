#include "primarylistdelegate.h"
#include <QPainter>
#include <qapplication.h>

PrimaryListDelegate::PrimaryListDelegate() {}

void PrimaryListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.text.clear();
    opt.state &= ~QStyle::State_HasFocus;

    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);

    QString left = index.data(Qt::DisplayRole).toString();
    QString right = index.data(Qt::UserRole).toString();

    QRect rect = option.rect.adjusted(4, 0, -4, 0);
    painter->setFont(option.font);
    painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, left);
    painter->drawText(rect, Qt::AlignRight | Qt::AlignVCenter, right);
}
