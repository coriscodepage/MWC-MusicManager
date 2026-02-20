#include "pastecommand.h"
#include "listitem.h"
#include <qfiledevice.h>
#include <qmimedata.h>

PasteCommand::PasteCommand(QListView *view, QByteArray &data, const QString &format, int row) : m_view(view), m_row(row), m_data(data), m_format(format) {
    QDataStream stream(&data, QIODevice::ReadOnly);
    m_count = 0;
    while(!stream.atEnd()) {
        QVariant item;
        stream >> item;
        m_count++;
    }

    if (row != -1) {
        auto model = view->model();
        auto index = model->index(row, 0);
        QModelIndexList list;
        list.append(index);
        auto mime = model->mimeData(list);
        m_oldData = mime->data(format);
    }
    m_row = row == -1 ? view->model()->rowCount() : row;
}

void PasteCommand::undo() {
    auto model = m_view->model();
    model->removeRows(m_row, m_count);
    if(!m_oldData.isEmpty()) {
        QMimeData mime;
        mime.setData(m_format, m_oldData);
        model->dropMimeData(&mime, Qt::CopyAction, m_row, 0, QModelIndex());
        m_view->setCurrentIndex(m_view->model()->index(m_row, 0));
    } else
        m_view->setCurrentIndex(m_view->model()->index(m_row - 1, 0));
}

void PasteCommand::redo() {
    auto model = m_view->model();
    QMimeData mime;
    mime.setData(m_format, m_data);
    if(!m_oldData.isEmpty()) {
        m_view->model()->removeRows(m_row, m_count);
    }
    model->dropMimeData(&mime, Qt::CopyAction, m_row, 0, QModelIndex());
    m_view->setCurrentIndex(m_view->model()->index(m_row, 0));
}
