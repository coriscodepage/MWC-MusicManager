#include "pastecommand.h"
#include "listitem.h"
#include <qfiledevice.h>
#include <qmimedata.h>

PasteCommand::PasteCommand(QAbstractItemModel *itemModel, QByteArray &data, const QString &format, int row) : m_itemModel(itemModel), m_row(row), m_data(data), m_format(format) {
    QDataStream stream(&data, QIODevice::ReadOnly);
    m_count = 0;
    while(!stream.atEnd()) {
        QVariant item;
        stream >> item;
        m_count++;
    }

    if (row != -1) {
        auto index = m_itemModel->index(row, 0);
        QModelIndexList list;
        list.append(index);
        auto mime = m_itemModel->mimeData(list);
        if (mime)
        {
            m_oldData = mime->data(format);
            delete mime;
        }
        if (format == "application/x-msc-list" && !m_oldData.isEmpty()) {
            QDataStream streamOld(&m_oldData, QIODevice::ReadOnly);
            QVector<bool> oldTypes;
            while (!streamOld.atEnd()) {
                QVariant v;
                streamOld >> v;
                ListItem item = v.value<ListItem>();
                oldTypes.append(item.type());
            }

            QDataStream streamNew(&m_data, QIODevice::ReadOnly);
            QByteArray encoded;
            QDataStream streamEnc(&encoded, QIODevice::WriteOnly);
            int i = 0;
            while (!streamNew.atEnd()) {
                QVariant v;
                streamNew >> v;
                ListItem item = v.value<ListItem>();
                item.setType(oldTypes.at(qMin(oldTypes.length() - 1, i)));
                streamEnc << QVariant::fromValue(item);
                i++;
            }
            m_data = encoded;
        }
    }
    m_row = row == -1 ? m_itemModel->rowCount() : row;
}

void PasteCommand::undo() {
    if (m_count <= 0)
        return;
    m_itemModel->removeRows(m_row, m_count);
    if(!m_oldData.isEmpty()) {
        QMimeData mime;
        mime.setData(m_format, m_oldData);
        m_itemModel->dropMimeData(&mime, Qt::CopyAction, m_row, 0, QModelIndex());
    }
}

void PasteCommand::redo() {
    if (m_count <= 0)
        return;
    QMimeData mime;
    mime.setData(m_format, m_data);
    if(!m_oldData.isEmpty()) {
        m_itemModel->removeRows(m_row, m_count);
    }
    m_itemModel->dropMimeData(&mime, Qt::CopyAction, m_row, 0, QModelIndex());
}
