#include "cutcommand.h"
#include <qapplication.h>
#include <qclipboard.h>
#include <qmimedata.h>

CutCommand::CutCommand(QListView *view, QByteArray &data, const QString &format): m_view(view), m_format(format) {
    m_indexes = m_view->selectionModel()->selectedIndexes();
    if (m_indexes.isEmpty())
        return;
    m_data = data;
}

void CutCommand::redo() {
    if (m_indexes.isEmpty() || m_data.isEmpty())
        return;

    auto model = m_view->model();

    QMimeData *mime = new QMimeData();
    mime->setData(m_format, m_data);
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setMimeData(mime);

    int firstRow = m_indexes.first().row();
    int rowCount = m_indexes.count();
    model->removeRows(firstRow, rowCount);
}

void CutCommand::undo() {
    if (m_indexes.isEmpty() || m_data.isEmpty())
        return;

    QMimeData mime;
    mime.setData(m_format, m_data);

    auto model = m_view->model();
    int firstRow = m_indexes.first().row();
    model->dropMimeData(&mime, Qt::CopyAction, firstRow, 0, QModelIndex());
}
