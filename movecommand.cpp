#include "movecommand.h"
#include <qabstractitemmodel.h>

MoveCommand::MoveCommand(CustomModelEdit *model, const QVector<QVariant> &movingItems, int sourceRow, int count, int destinationChild)
    : m_model(model)
    , m_movingItems(movingItems)
    , m_sourceRow(sourceRow)
    , m_count(count)
    , m_destinationChild(destinationChild)
{}

void MoveCommand::undo() {
    if (m_destinationChild > m_sourceRow) {
        m_model->moveInternal(m_movingItems, m_destinationChild - m_count, m_count, m_sourceRow);
    } else {
        m_model->moveInternal(m_movingItems, m_destinationChild, m_count, m_sourceRow + m_count);
    }
}

void MoveCommand::redo() {
    m_model->moveInternal(m_movingItems, m_sourceRow, m_count, m_destinationChild);
}
