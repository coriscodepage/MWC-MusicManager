#include "pastecommand.h"

PasteCommand::PasteCommand(QListView *view, const QMimeData *data, int row) : m_view(view), m_data(data), m_row(row) {}

void PasteCommand::undo() {
    auto model = m_view->model();
    model->removeRow(m_row);
}

void PasteCommand::redo() {
    auto model = m_view->model();
    model->dropMimeData(m_data, Qt::CopyAction, m_row, 0, QModelIndex());
    if (m_row == -1)
        m_row = model->rowCount() - 1;
    m_view->setCurrentIndex(model->index(m_row, 0));
}
