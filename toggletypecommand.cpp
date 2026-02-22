#include "toggletypecommand.h"

ToggleTypeCommand::ToggleTypeCommand(PrimaryListModel *model, const QModelIndex &index) : m_model(model), m_index(index) {}

void ToggleTypeCommand::redo() {
    m_model->setType(m_index.row(), !m_model->getItem(m_index).type());
}

void ToggleTypeCommand::undo() {
    redo();
}
