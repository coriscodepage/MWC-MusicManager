#include "insertprimarycommand.h"
#include "listitem.h"

InsertPrimaryCommand::InsertPrimaryCommand(QListView *view, PrimaryListModel *model, const QString &title, bool type, int row): m_view(view), m_model(model), m_title(title), m_type(type), m_row(row), m_edit(true) {}

void InsertPrimaryCommand::undo() {
    m_model->removeRow(m_row);
}

void InsertPrimaryCommand::redo() {
    m_view->setFocus();
    m_model->addItem(ListItem(m_title, m_type), m_row);
    QModelIndex index = m_model->index(m_row, 0);
    if (m_edit) {
        m_view->edit(index);
        m_edit = false;
    }
    m_view->setCurrentIndex(index);
}
