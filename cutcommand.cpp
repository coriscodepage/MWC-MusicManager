#include "cutcommand.h"
#include <qapplication.h>
#include <qclipboard.h>
#include <qmimedata.h>

CutCommand::CutCommand(QListView *view): m_view(view) {
    auto model = m_view->model();
    m_indexes = m_view->selectionModel()->selectedIndexes();
    if (m_indexes.isEmpty())
        return;
    m_mimeData = model->mimeData(m_indexes);
}

void CutCommand::redo() {
    if (m_indexes.isEmpty() || m_mimeData == nullptr)
        return;

    auto model = m_view->model();

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setMimeData(m_mimeData);

    int firstRow = m_indexes.first().row();
    int rowCount = m_indexes.count();
    model->removeRows(firstRow, rowCount);
}

void CutCommand::undo() {
    if (m_indexes.isEmpty() || m_mimeData == nullptr)
        return;

    auto model = m_view->model();
    int firstRow = m_indexes.first().row();
    int rowCount = m_indexes.count();
    model->insertRows(firstRow, rowCount);
    model->dropMimeData(m_mimeData, Qt::CopyAction, firstRow, 0, QModelIndex());
}
