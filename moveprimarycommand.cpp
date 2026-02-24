#include "moveprimarycommand.h"
#include "listitem.h"
#include <qabstractitemmodel.h>

MovePrimaryCommand::MovePrimaryCommand(PrimaryListModel *model, const QVector<ListItem> &movingItems, int sourceRow, int count, int destinationChild)
    : m_model(model)
    , m_movingItems(movingItems)
    , m_sourceRow(sourceRow)
    , m_count(count)
    , m_destinationChild(destinationChild)
{}

void MovePrimaryCommand::undo() {
    if (m_destinationChild > m_sourceRow) {
        m_model->moveInternal(m_movingItems, m_destinationChild - m_count, m_count, m_sourceRow);
    } else {
        m_model->moveInternal(m_movingItems, m_destinationChild, m_count, m_sourceRow + m_count);
    }
}

void MovePrimaryCommand::redo() {
    m_model->moveInternal(m_movingItems, m_sourceRow, m_count, m_destinationChild);
}
