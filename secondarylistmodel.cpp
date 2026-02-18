#include "secondarylistmodel.h"
#include <QMimeData>
#include <qitemselectionmodel.h>

SecondaryListModel::SecondaryListModel(QObject *parent, MusicStorage *musicStore)
    : QAbstractListModel{parent}
    , m_musicStore(musicStore)
{}

int SecondaryListModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid() || m_item == nullptr) return 0;
    return m_item->itemCount();
}

int SecondaryListModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 1;
}

QVariant SecondaryListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_item->itemCount() || m_item == nullptr)
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case 0:
            return m_item->getItem(index.row())->title();
            break;
        default:
            break;
        }
    }

    return QVariant();
}

Qt::ItemFlags SecondaryListModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::ItemIsDropEnabled | Qt::NoItemFlags;

    return Qt::ItemIsSelectable
           | Qt::ItemIsEnabled
           | Qt::ItemIsEditable
           | Qt::ItemIsDragEnabled
           | Qt::ItemIsDropEnabled;
}

bool SecondaryListModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role == Qt::EditRole && m_item != nullptr) {
        m_item->getItem(index.row())->setTitle(value.toString());
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}

bool SecondaryListModel::removeRows(int row, int count, const QModelIndex &parent) {
    Q_UNUSED(parent)
    if (m_item == nullptr || row < 0 || row >= m_item->itemCount() || count <= 0) return false;
    qDebug() << QString("[SecondaryListModel] Removing %1 row(s) starting from %2 in %3 list").arg(count).arg(row).arg(m_item->title());
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = row + count - 1; i >= row; i--) {
        QString hash = m_item->getItem(i)->getHash();
        m_item->removeItem(i);
        m_musicStore->checkedRemoveSong(hash);
    }
    endRemoveRows();
    return true;
}

bool SecondaryListModel::insertRows(int row, int count, const QModelIndex &parent) {
    Q_UNUSED(parent)

    if (m_item == nullptr || row < 0 || row > m_item->itemCount() || count <= 0) return false;
    qDebug() << QString("[SecondaryListModel] Inserting %1 row(s) starting from %2 in %3 list").arg(count).arg(row).arg(m_item->title());
    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int i = row; i < row + count; i++)
        m_item->addItem(MusicItem("New song", nullptr), i);
    endInsertRows();
    return true;
}


void SecondaryListModel::addItemAt(const MusicItem &item, int row) {
    if (m_item == nullptr) return;
    if (row < 0) row = m_item->itemCount();
    beginInsertRows(QModelIndex(), row, row);
    m_item->addItem(item, row);
    endInsertRows();
}

void SecondaryListModel::setSource(ListItem *item) {
    beginResetModel();
    m_item = item;
    endResetModel();
}

ListItem *SecondaryListModel::getItem() {
    return m_item;
}

Qt::DropActions SecondaryListModel::supportedDropActions() const {
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList SecondaryListModel::mimeTypes() const {
    return { "application/x-msc-song" };
}

QMimeData *SecondaryListModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mime = new QMimeData;

    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);

    for (const QModelIndex &index : indexes) {
        if (!index.isValid())
            continue;

        for (auto &index : indexes)
            stream << *(m_item->getItem(index.row()));
    }
    m_draggedIndexes = indexes;
    mime->setData("application/x-msc-song", encoded);
    return mime;
}

bool SecondaryListModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    if (!data->hasFormat("application/x-msc-song") || m_musicStore==nullptr)
        return false;

    QByteArray encoded = data->data("application/x-msc-song");
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    QItemSelection selection;

    while (!stream.atEnd()) {
        MusicItem item;
        stream >> item;
        QString hash = item.getHash();
        auto songPtr = m_musicStore->queryMusic(hash);
        item.setSong(songPtr);
        addItemAt(item, row);
    }
    return true;
}

const QModelIndexList &SecondaryListModel::copiedIndexes() const {
    return m_draggedIndexes;
}

bool SecondaryListModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const {
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)

    if (!data)
        return false;

    if (!data->hasFormat("application/x-msc-song"))
        return false;

    if (action != Qt::CopyAction && action != Qt::MoveAction)
        return false;

    if (data->data("application/x-msc-song").isEmpty())
        return false;

    return true;
}

