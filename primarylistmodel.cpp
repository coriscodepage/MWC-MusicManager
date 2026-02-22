#include "primarylistmodel.h"
#include "moveprimarycommand.h"
#include <QMimeData>
#include <qitemselectionmodel.h>

PrimaryListModel::PrimaryListModel(QObject *parent, MusicStorage *musicStore, QUndoStack *undoStack)
    : QAbstractListModel{parent}
    , m_musicStore(musicStore)
    , m_undoStack(undoStack)
{}

int PrimaryListModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_items.length();
}

int PrimaryListModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent);
    return 1;
}

QVariant PrimaryListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_items.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0:
            return m_items[index.row()].title();
            break;
        default:
            break;
        }
    } else if (role == Qt::EditRole) {
        return m_items[index.row()].title();
    } else if (role == Qt::UserRole) {
        return m_items[index.row()].type() ? "CD" : "Radio";
    }

    return QVariant();
}

Qt::ItemFlags PrimaryListModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::ItemIsDropEnabled | Qt::NoItemFlags;

    return Qt::ItemIsSelectable
           | Qt::ItemIsEnabled
           | Qt::ItemIsEditable
           | Qt::ItemIsDragEnabled
           | Qt::ItemIsDropEnabled;
}

bool PrimaryListModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role == Qt::EditRole) {
        m_items[index.row()].setTitle(value.toString());
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}

void PrimaryListModel::setType(int index, bool type) {
    if(index < 0 || index > m_items.count()) return;
    bool oldType = m_items[index].type();
    m_items[index].setType(type);
    if(type != oldType) {
        emit typeChanged();
        QModelIndex idx = this->index(index, 0);
        emit dataChanged(idx, idx);
    };
}

ListItem &PrimaryListModel::getItem(const QModelIndex &index) {
    return m_items[index.row()];
}

void PrimaryListModel::addItem(const ListItem &item, int index) {
    if (index == -1) {
        beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
        m_items.append(item);
        endInsertRows();
    } else {
        beginInsertRows(QModelIndex(), index, index);
        m_items.insert(index, item);
        endInsertRows();
    }
}

void PrimaryListModel::removeItem(const QModelIndex &index) {
    int row = index.row(); // FIXME
    if (row < 0 || row >= m_items.size()) return;
    beginRemoveRows(QModelIndex(), row, row);
    for(auto &item : m_items[row].getItems()) {
        item.setSong(nullptr);
    }
    m_items.removeAt(row);
    endRemoveRows();
}

const QVector<ListItem> &PrimaryListModel::getItems() const {
    return m_items;
}

QVector<ListItem> &PrimaryListModel::getItems() {
    return m_items;
}

void PrimaryListModel::setItems(const QVector<ListItem> &items) {
    beginResetModel();
    m_items = items;
    endResetModel();
}

void  PrimaryListModel::insertAt(const ListItem &item, int index) {
    if (index >= 0) {
        beginInsertRows(QModelIndex(), index, index);
        m_items.insert(index, item);
        endInsertRows();
    } else {
        beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
        m_items.append(item);
        endInsertRows();
    }
}

QStringList PrimaryListModel::mimeTypes() const {
    return { "application/x-msc-list" };
}

QMimeData* PrimaryListModel::mimeData(const QModelIndexList &indexes) const {
    QMimeData *mime = new QMimeData;

    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);
    for (const QModelIndex &index : indexes) {
        if (!index.isValid())
            continue;
        stream << QVariant::fromValue(m_items[index.row()]);
    }
    m_draggedIndexes = indexes;
    mime->setData("application/x-msc-list", encoded);
    return mime;
}

bool PrimaryListModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const {
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)

    if (!data)
        return false;

    if (!data->hasFormat("application/x-msc-list"))
        return false;

    if (action != Qt::CopyAction && action != Qt::MoveAction)
        return false;

    if (data->data("application/x-msc-list").isEmpty())
        return false;

    return true;
}

bool PrimaryListModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) {
    if (!data->hasFormat("application/x-msc-list"))
        return false;

    QByteArray encoded = data->data("application/x-msc-list");
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    QItemSelection selection;

    while (!stream.atEnd()) {
        QVariant v;
        stream >> v;
        ListItem item = v.value<ListItem>();
        for (auto &s : item.getItems()) {
            QString hash = s.getHash();
            auto songPtr = m_musicStore->queryMusic(hash);
            s.setSong(songPtr);
        }
        insertAt(item, row );
    }
    return true;
}

Qt::DropActions PrimaryListModel::supportedDropActions() const {
    return Qt::CopyAction | Qt::MoveAction;
}

bool PrimaryListModel::removeRows(int row, int count, const QModelIndex &parent) {
    Q_UNUSED(parent)
    if (row < 0 || row >= m_items.size() || count <= 0) return false;
    qDebug() << QString("[PrimaryListModel] Removing %1 row(s) starting from %2 in primary list").arg(count).arg(row);
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = row + count - 1; i >= row; i--) {
        for (auto &song : m_items.at(i).getItems()) {
            m_musicStore->checkedRemoveSong(song.getHash());
        }
        m_items.removeAt(i);
    }
    endRemoveRows();
    return true;
}

bool PrimaryListModel::insertRows(int row, int count, const QModelIndex &parent) {
    Q_UNUSED(parent)

    if (row < 0 || row > m_items.size() || count <= 0) return false;
    qDebug() << QString("[PrimaryListModel] Inserting %1 row(s) starting from %2 in primary list").arg(count).arg(row);
    beginInsertRows(QModelIndex(), row, row + count - 1);
    for (int i = row; i < row + count; i++)
        m_items.insert(i, ListItem("New list"));
    endInsertRows();
    return true;
}

bool PrimaryListModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) {
    Q_UNUSED(sourceParent)
    Q_UNUSED(destinationParent)

    if (sourceRow < 0 || sourceRow + count > rowCount() || destinationChild < 0 || destinationChild > rowCount())
        return false;

    beginMoveRows(QModelIndex(), sourceRow, sourceRow + count - 1, QModelIndex(), destinationChild);
    QVector<ListItem> movingItems;
    for (int i = 0; i < count; ++i)
        movingItems.append(m_items[sourceRow]);

    MovePrimaryCommand *cmd = new MovePrimaryCommand(this, movingItems, sourceRow, count, destinationChild);
    m_undoStack->push(cmd);
    return true;
}

void PrimaryListModel::moveInternal(const QVector<ListItem> &movingItems, int sourceRow, int count, int destinationChild) {
    beginMoveRows(QModelIndex(), sourceRow, sourceRow + count - 1, QModelIndex(), destinationChild);

    for (int i = 0; i < count; ++i)
        m_items.removeAt(sourceRow);

    int insertPos = destinationChild;
    if (destinationChild > sourceRow) insertPos -= count;

    for (int i = 0; i < movingItems.size(); ++i)
        m_items.insert(insertPos + i, movingItems[i]);
    endMoveRows();
}
