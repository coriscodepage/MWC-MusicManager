#include "targetlistview.h"
#include <qevent.h>

void TargetListView::dragEnterEvent(QDragEnterEvent *event)
{
    setFocus();
    event->accept();
}

void TargetListView::dragMoveEvent(QDragMoveEvent *event)
{
    QModelIndex hovered = indexAt(event->position().toPoint());
    if (hovered.isValid()) {
        selectionModel()->setCurrentIndex(
            hovered,
            QItemSelectionModel::ClearAndSelect
            );
    }

    if (event->source() == this) {
        QListView::dragMoveEvent(event);
    } else {
        event->setDropAction(Qt::IgnoreAction);
        event->accept();
    }
}

void TargetListView::dragLeaveEvent(QDragLeaveEvent *event)
{
    selectionModel()->clearCurrentIndex();
    QListView::dragLeaveEvent(event);
}

void TargetListView::dropEvent(QDropEvent *event)
{
    if (event->source() == this)
        QListView::dropEvent(event);
    else
        event->ignore();
}
