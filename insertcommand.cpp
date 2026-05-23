#include "insertcommand.h"

InsertCommand::InsertCommand(CustomModelEdit *model, const QString &title, bool type, int row, QUndoCommand *parent): QUndoCommand(parent), m_model(model), m_title(title), m_type(type), m_row(row) {}

void InsertCommand::undo() {
    m_model->removeAt(m_row);
}

void InsertCommand::redo() {
    m_model->insertEmptyAt(m_row, m_title, m_type);
}
