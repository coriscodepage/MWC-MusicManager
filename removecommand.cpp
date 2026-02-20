#include "removecommand.h"
#include <qmimedata.h>

RemoveCommand::RemoveCommand(QListView *view, int row, SecondaryListModel *secondaryModel, QUndoCommand *parent): QUndoCommand(parent), m_view(view), m_row(row), m_secondaryModel(secondaryModel) {
    auto model = view->model();
    auto index = model->index(row, 0);
    QModelIndexList list;
    list.append(index);
    auto mime = model->mimeData(list);
    QString format = model->mimeTypes().constFirst();
    m_data = mime->data(format);
    m_format = format;
    delete mime;
}

void RemoveCommand::redo() {
    if (m_secondaryModel != nullptr) {
        m_secondaryModel->removeRows(0, m_secondaryModel->rowCount());
    }
    m_view->model()->removeRow(m_row);
}

void RemoveCommand::undo() {
    QMimeData mime;
    mime.setData(m_format, m_data);
    m_view->model()->dropMimeData(&mime, Qt::CopyAction, m_row, 0, QModelIndex());
    m_view->setCurrentIndex(m_view->model()->index(m_row, 0));
}
