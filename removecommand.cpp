#include "removecommand.h"
#include <qmimedata.h>

RemoveCommand::RemoveCommand(CustomModelEdit *itemModel, QModelIndexList indexes, QUndoCommand *parent): QUndoCommand(parent), m_itemModel(itemModel), m_indexes(indexes){
    std::sort(m_indexes.begin(), m_indexes.end(), [](QModelIndex &a, QModelIndex &b) {return a.row() < b.row();});
    m_data = m_itemModel->getSelf(m_indexes);
    // auto index = m_itemModel->index(row, 0);
    // QModelIndexList list;
    // list.append(index);
    // auto mime = m_itemModel->mimeData(list);
    // QString format = m_itemModel->mimeTypes().constFirst();
    // m_data = mime->data(format);
    // m_format = format;
    // delete mime;
}

void RemoveCommand::redo() {
    for (int i = m_indexes.length() - 1; i >= 0; i--) {
        const auto &index = m_indexes[i];
        m_itemModel->removeAt(index.row());
    }
    // if (m_secondaryModel != nullptr) {
    //     m_secondaryModel->removeRows(0, m_secondaryModel->rowCount());
    // }
    // m_itemModel->removeRow(m_row);
}

void RemoveCommand::undo() {
    m_itemModel->restoreEntry(m_data, {}, m_indexes);
    // QMimeData mime;
    // mime.setData(m_format, m_data);
    // m_itemModel->dropMimeData(&mime, Qt::CopyAction, m_row, 0, QModelIndex());
}
