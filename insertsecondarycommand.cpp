#include "insertsecondarycommand.h"

InsertSecondaryCommand::InsertSecondaryCommand(QListView *view, int row): m_view(view), m_row(row) {}

void InsertSecondaryCommand::undo() {
    auto model = m_view->model();
    model->removeRow(m_row);
    QModelIndex index = model->index(qMax(0, m_row - 1), 0);
    m_view->setCurrentIndex(index);
    m_view->setFocus();}

void InsertSecondaryCommand::redo() {
    auto model = m_view->model();
    model->insertRow(m_row);
    QModelIndex index = model->index(m_row, 0);
    m_view->setCurrentIndex(index);
    m_view->setFocus();
}
