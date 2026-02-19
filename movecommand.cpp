#include "movecommand.h"
#include "musicitem.h"

MoveCommand::MoveCommand(QAbstractListModel *model, const QMimeData *data, int row, int count, const QModelIndexList &draggedIndexes)
    : m_model(model)
    , m_data(data)
    , m_draggedIndexes(draggedIndexes)
    , m_count(count)
{
    if (row < 0) m_row = model->rowCount();
    else m_row = row;
}

void MoveCommand::redo() {
    m_model->dropMimeData(m_data, Qt::CopyAction, m_row, 0, QModelIndex());
}

void MoveCommand::undo() {
    int adjustedTo = m_row >= m_draggedIndexes.first().row() ? m_row + m_count : m_row;
    m_model->removeRows(adjustedTo, m_count);
    m_model->insertRows(m_draggedIndexes.first().row(), m_count);
    m_model->dropMimeData(m_data, Qt::CopyAction, m_draggedIndexes.first().row(), 0, QModelIndex());
}
