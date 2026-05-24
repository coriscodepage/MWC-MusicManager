#include "cutcommand.h"
#include <qapplication.h>
#include <qclipboard.h>
#include <qmimedata.h>

CutCommand::CutCommand(QAbstractItemModel *itemModel, QByteArray &data, const QString &format, const QModelIndexList &selectedindexes): m_itemModel(itemModel), m_format(format), m_indexes(selectedindexes) {
    if (m_indexes.isEmpty())
        return;
    m_data = data;
}

void CutCommand::redo() {
    if (m_indexes.isEmpty() || m_data.isEmpty())
        return;

    QMimeData *mime = new QMimeData();
    mime->setData(m_format, m_data);
    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setMimeData(mime);

    int firstRow = m_indexes.first().row();
    int rowCount = m_indexes.count();
    m_itemModel->removeRows(firstRow, rowCount);
}

void CutCommand::undo() {
    if (m_indexes.isEmpty() || m_data.isEmpty())
        return;

    QMimeData mime;
    mime.setData(m_format, m_data);

    int firstRow = m_indexes.first().row();
    m_itemModel->dropMimeData(&mime, Qt::CopyAction, firstRow, 0, QModelIndex());
}
