#include "cutcommand.h"
#include "removecommand.h"
#include <qapplication.h>
#include <qclipboard.h>
#include <qmimedata.h>

CutCommand::CutCommand(CustomModelEdit *itemModel, QByteArray &data, const QString &format, const QModelIndexList &selectedindexes): m_itemModel(itemModel), m_format(format), m_indexes(selectedindexes) {
    m_removeCommand = new RemoveCommand(itemModel, selectedindexes, this);
    if (m_indexes.isEmpty())
        return;
    std::sort(m_indexes.begin(), m_indexes.end(), [](QModelIndex &a, QModelIndex &b) {return a.row() < b.row();});
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
    m_removeCommand->redo();
}

void CutCommand::undo() {
    if (m_indexes.isEmpty() || m_data.isEmpty())
        return;
    m_removeCommand->undo();
}
