#include "secondarylistmodel.h"
#include "edittitlecommand.h"
#include "movecommand.h"
#include <QMimeData>
#include <qitemselectionmodel.h>
#include <qpixmap.h>

SecondaryListModel::SecondaryListModel(MusicStorage *musicStore, QUndoStack *undoStack, QObject *parent)
    : QAbstractListModel{parent}
    , m_musicStore(musicStore)
    , m_undoStack(undoStack)
{}

int SecondaryListModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid() || m_list == nullptr) return 0;
    return m_list->itemCount();
}

int SecondaryListModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 1;
}

QVariant SecondaryListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_list->itemCount() || m_list == nullptr)
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case 0:
            return m_list->getItem(index.row())->title();
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
    if (role == Qt::EditRole && m_list != nullptr) {
        auto *cmd = new EditTitleCommand(this, 0, value.toString(), index);
        m_undoStack->push(cmd);
        return true;
    }
    return false;
}

bool SecondaryListModel::removeRows(int row, int count, const QModelIndex &parent) {
    Q_UNUSED(parent)
    if (m_list == nullptr || row < 0 || row >= m_list->itemCount() || count <= 0) return false;
    qDebug() << QString("[SecondaryListModel] Removing %1 row(s) starting from %2 in %3 list").arg(count).arg(row).arg(m_list->title());
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = row + count - 1; i >= row; i--) {
        QString hash = m_list->getItem(i)->getHash();
        m_list->removeItem(i);
        m_musicStore->checkedRemoveSong(hash);
    }
    endRemoveRows();
    return true;
}

bool SecondaryListModel::insertRows(int row, int count, const QModelIndex &parent) {
    Q_UNUSED(parent)

    if (m_list == nullptr || row < 0 || row > m_list->itemCount() || count <= 0) return false;
    qDebug() << QString("[SecondaryListModel] Inserting %1 row(s) starting from %2 in %3 list").arg(count).arg(row).arg(m_list->title());
    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int i = row; i < row + count; i++)
        m_list->addItem(MusicItem("New song", nullptr), i);
    endInsertRows();
    return true;
}

bool SecondaryListModel::insertRowInternal(int row, MusicItem &item) {
    if (m_list == nullptr || row < 0 || row > m_list->itemCount()) return false;
    qDebug() << QString("[SecondaryListModel] Inserting row with data starting from %1 in %2 list").arg(row).arg(m_list->title());
    beginInsertRows(QModelIndex(), row, row);
    m_list->addItem(item, row);
    endInsertRows();
    return true;
}

void SecondaryListModel::addItemAt(const MusicItem &item, int row) {
    if (m_list == nullptr) return;
    if (row < 0) row = m_list->itemCount();
    beginInsertRows(QModelIndex(), row, row);
    m_list->addItem(item, row);
    endInsertRows();
}

void SecondaryListModel::setSource(ListItem *item) {
    beginResetModel();
    m_list = item;
    endResetModel();
}

ListItem *SecondaryListModel::getItem() {
    return m_list;
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

        stream << QVariant::fromValue(*(m_list->getItem(index.row())));
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

    while (!stream.atEnd()) {
        QVariant v;
        stream >> v;
        MusicItem item = v.value<MusicItem>();
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

bool SecondaryListModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) {
    if (sourceRow < 0 || sourceRow + count > rowCount() || destinationChild < 0 || destinationChild > rowCount())
        return false;

    QVector<QVariant> movingItems;
    for (int i = 0; i < count; ++i)
        movingItems.append(QVariant::fromValue(m_list->getItems().at(sourceRow)));

    MoveCommand *cmd = new MoveCommand(this, movingItems, sourceRow, count, destinationChild);
    m_undoStack->push(cmd);
    return true;
}

void SecondaryListModel::moveInternal(const QVector<QVariant> &movingItems, int sourceRow, int count, int destinationChild) {
    beginMoveRows(QModelIndex(), sourceRow, sourceRow + count - 1, QModelIndex(), destinationChild);

    for (int i = 0; i < count; ++i)
        m_list->getItems().removeAt(sourceRow);

    int insertPos = destinationChild;
    if (destinationChild > sourceRow) insertPos -= count;

    for (int i = 0; i < movingItems.size(); ++i) {
        if (movingItems[i].canConvert<MusicItem>()) {
            const auto &item = qvariant_cast<MusicItem>(movingItems[i]);
            m_list->getItems().insert(insertPos + i, item);
        }
    }
    endMoveRows();
}

// QString SecondaryListModel::getTitle(int index) const {
//     if (m_item == nullptr || index < 0 || index >= m_item->itemCount()) return {};
//     QString title = m_item->getItem(index)->title();
//     return title;
// }

// QString SecondaryListModel::getArtist(int index) const {
//     if (m_item == nullptr || index < 0 || index >= m_item->itemCount()) return {};
//     QString artist = m_item->getItem(index)->artist();
//     return artist;
// }

// QString SecondaryListModel::getHash(int index) const {
//     if (m_item == nullptr || index < 0 || index >= m_item->itemCount()) return {};
//     QString hash = m_item->getItem(index)->getHash();
//     return hash;
// }

// void SecondaryListModel::setHash(const QString &hash, int index) {
//     if (m_item == nullptr || index < 0 || index >= m_item->itemCount()) return;
//     m_item->getItem(index)->setHash(hash);
// }

// QPixmap SecondaryListModel::getPixmap(int index) const {
//     static const QPixmap empty(":/defaults/no-image.webp");
//     if (m_item == nullptr || index < 0 || index >= m_item->itemCount()) return empty;
//     MusicItem *item = m_item->getItem(index);
//     if (!item->hasThumbnail()) return empty;
//     QString path = item->pixmapPath();
//     return QPixmap(path);
// }

// QString SecondaryListModel::getSongPath(int index) const {
//     if (m_list == nullptr || index < 0 || index >= m_list->itemCount()) return QString();
//     QString path = m_list->getItem(index)->songPath();
//     return path;
// }

const QVector<MusicItem> &SecondaryListModel::getSongs() const {
    static const QVector<MusicItem> empty;
    if (m_list == nullptr) return empty;
    return m_list->getItems();
}

// const QString SecondaryListModel::getInsertHash() const {
//     if (m_list == nullptr) return QString();
//     return m_list->getInsertHash();
// }

const MusicItem* SecondaryListModel::getSong(int row) {
    static auto nullObject = new MusicItem();
    if (m_list == nullptr) return nullObject;
    if (row < 0 || row >= m_list->itemCount()) return nullObject;
    return m_list->getItem(row);
}

void SecondaryListModel::setField(int field, const QString &value, const QModelIndex &index) {
    if (m_list == nullptr) return;
    auto *item = m_list->getItem(index.row());
    switch (field) {
    case 0:
        item->setTitle(value);
        break;
    case 1:
        if (item->hasSong())
            item->setArtist(value);
        break;
    }
    emit dataChanged(index, index, {Qt::EditRole});
}

QString SecondaryListModel::getField(int field, const QModelIndex &index) {
    if (m_list == nullptr) return {};
    auto *item = m_list->getItem(index.row());
    switch (field) {
    case 0:
        return item->title();
        break;
    case 1:
        if (item->hasSong())
            return item->artist();
        break;
    }
    return {};
}

void SecondaryListModel::removeAt(int row) {
    removeRow(row);
}

void SecondaryListModel::insertEmptyAt(int row, const QString &name, bool type) {
    addItemAt(MusicItem(name, nullptr), row);
}