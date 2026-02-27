#include "targetlistview.h"
#include <qevent.h>

void TargetListView::dragEnterEvent(QDragEnterEvent *event)
{
    setFocus();
    event->accept();
}

void TargetListView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->source() == this) {
        event->setDropAction(Qt::MoveAction);
        QListView::dragMoveEvent(event);
    } else {
        QModelIndex hovered = indexAt(event->position().toPoint());
        if (hovered.isValid())
            selectionModel()->setCurrentIndex(hovered, QItemSelectionModel::ClearAndSelect);
        event->ignore();
    }
}

void TargetListView::dragLeaveEvent(QDragLeaveEvent *event)
{
    QListView::dragLeaveEvent(event);
}

void TargetListView::dropEvent(QDropEvent *event)
{
    if (event->source() == this)
        QListView::dropEvent(event);
    else
        event->ignore();
}
