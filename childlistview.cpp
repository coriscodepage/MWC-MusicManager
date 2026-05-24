#include "childlistview.h"
#include <qevent.h>

void ChildListView::dragEnterEvent(QDragEnterEvent *event)
{
    if (m_forcedCopy)
        event->setDropAction(Qt::CopyAction);
    QListView::dragEnterEvent(event);
}

void ChildListView::dragMoveEvent(QDragMoveEvent *event)
{
    if (m_forcedCopy)
        event->setDropAction(Qt::CopyAction);
    QListView::dragMoveEvent(event);
}

void ChildListView::dropEvent(QDropEvent *event)
{
    DropMacroGuard guard(dynamic_cast<CustomModelEdit*>(model()), tr("Drop"));
    QListView::dropEvent(event);
    if (m_forcedCopy)
        event->setDropAction(Qt::CopyAction);
    m_forcedCopy = false;
    event->accept();
}

void ChildListView::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_forcedCopy = false;
    emit forceCopy(m_forcedCopy);
    QListView::dragLeaveEvent(event);
}

void ChildListView::setForceCopy(bool state) {
    m_forcedCopy = state;
    emit forceCopy(m_forcedCopy);
}