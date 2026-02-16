#include "secondarylistmodel.h"

SecondaryListModel::SecondaryListModel(QObject *parent)
    : QAbstractListModel{parent}
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
            return m_item->getItem(index.row()).title();
            break;
        default:
            break;
        }
    }

    return QVariant();
}

Qt::ItemFlags SecondaryListModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsSelectable
           | Qt::ItemIsEnabled
           | Qt::ItemIsEditable;
}

bool SecondaryListModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role == Qt::EditRole && m_item != nullptr) {
        m_item->getItem(index.row()).setTitle(value.toString());
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}

void SecondaryListModel::addItem(const MusicItem &item) {
    if (m_item == nullptr) return;
    beginInsertRows(QModelIndex(), m_item->itemCount(), m_item->itemCount());
    m_item->addItem(item);
    endInsertRows();
}

void SecondaryListModel::removeItem(int row) {
    if (row < 0 || row >= m_item->itemCount() || m_item == nullptr) return;
    beginRemoveRows(QModelIndex(), row, row);
    m_item->removeItem(row);
    endRemoveRows();
}

void SecondaryListModel::setSource(ListItem *item) {
    beginResetModel();
    m_item = item;
    endResetModel();
}
