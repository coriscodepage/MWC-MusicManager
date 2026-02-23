#include "edittitlecommand.h"

EditTitleCommand::EditTitleCommand(TextFieldEdit *model, int field, const QString &value, const QModelIndex &index): m_model(model), m_field(field), m_value(value), m_index(index) {
    m_oldValue = m_model->getField(m_field, m_index);
}

void EditTitleCommand::undo() {
    m_model->setField(m_field, m_oldValue, m_index);
}

void EditTitleCommand::redo() {
    m_model->setField(m_field, m_value, m_index);
}
