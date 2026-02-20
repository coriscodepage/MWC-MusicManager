#include "secondarylistmodel.h"
#include "movecommand.h"
#include "movesecondarycommand.h"
#include <QMimeData>
#include <qitemselectionmodel.h>
#include <qpixmap.h>

SecondaryListModel::SecondaryListModel(QObject *parent, MusicStorage *musicStore, QUndoStack *undoStack)
    : QAbstractListModel{parent}
    , m_musicStore(musicStore)
    , m_undoStack(undoStack)
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

        stream << QVariant::fromValue(*(m_item->getItem(index.row())));
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

    QVector<MusicItem> movingItems;
    for (int i = 0; i < count; ++i)
        movingItems.append(m_item->getItems().at(sourceRow));

    MoveSecondaryCommand *cmd = new MoveSecondaryCommand(this, movingItems, sourceRow, count, destinationChild);
    m_undoStack->push(cmd);
    return true;
}

void SecondaryListModel::moveInternal(const QVector<MusicItem> &movingItems, int sourceRow, int count, int destinationChild) {
    beginMoveRows(QModelIndex(), sourceRow, sourceRow + count - 1, QModelIndex(), destinationChild);

    for (int i = 0; i < count; ++i)
        m_item->getItems().removeAt(sourceRow);

    int insertPos = destinationChild;
    if (destinationChild > sourceRow) insertPos -= count;

    for (int i = 0; i < movingItems.size(); ++i)
        m_item->getItems().insert(insertPos + i, movingItems[i]);
    endMoveRows();
}

QString SecondaryListModel::getTitle(int index) const {
    if (m_item == nullptr || index < 0 || index >= m_item->itemCount()) return {};
    QString title = m_item->getItem(index)->title();
    return title;
}

QString SecondaryListModel::getHash(int index) const {
    if (m_item == nullptr || index < 0 || index >= m_item->itemCount()) return {};
    QString hash = m_item->getItem(index)->getHash();
    return hash;
}

void SecondaryListModel::setHash(const QString &hash, int index) {
    if (m_item == nullptr || index < 0 || index >= m_item->itemCount()) return;
    m_item->getItem(index)->setHash(hash);
}

QPixmap SecondaryListModel::getPixmap(int index) const {
    static const QPixmap empty(":/defaults/no-image.webp");
    if (m_item == nullptr || index < 0 || index >= m_item->itemCount()) return empty;
    MusicItem *item = m_item->getItem(index);
    if (!item->hasThumbnail()) return empty;
    QString path = item->pixmapPath();
    return QPixmap(path);
}

QString SecondaryListModel::getSongPath(int index) const {
    if (m_item == nullptr || index < 0 || index >= m_item->itemCount()) return QString();
    QString path = m_item->getItem(index)->songPath();
    return path;
}

const QVector<MusicItem> &SecondaryListModel::getSongs() const {
    static const QVector<MusicItem> empty;
    if (m_item == nullptr) return empty;
    return m_item->getItems();
}
