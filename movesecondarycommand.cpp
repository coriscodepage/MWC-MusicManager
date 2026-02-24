#include "movesecondarycommand.h"

MoveSecondaryCommand::MoveSecondaryCommand(SecondaryListModel *model, const QVector<MusicItem> &movingItems, int sourceRow, int count, int destinationChild)
    : m_model(model)
    , m_movingItems(movingItems)
    , m_sourceRow(sourceRow)
    , m_count(count)
    , m_destinationChild(destinationChild)
{}

void MoveSecondaryCommand::undo() {
    if (m_destinationChild > m_sourceRow) {
        m_model->moveInternal(m_movingItems, m_destinationChild - m_count, m_count, m_sourceRow);
    } else {
        m_model->moveInternal(m_movingItems, m_destinationChild, m_count, m_sourceRow + m_count);
    }
}

void MoveSecondaryCommand::redo() {
    m_model->moveInternal(m_movingItems, m_sourceRow, m_count, m_destinationChild);
}
