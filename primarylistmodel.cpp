#include "primarylistmodel.h"

PrimaryListModel::PrimaryListModel(QObject *parent)
    : QAbstractListModel{parent}
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
            return m_items[index.row()].title() + "    " + (m_items[index.row()].type() ? "CD" : "Radio");
            break;
        default:
            break;
        }
    } else if (role == Qt::EditRole) {
        return m_items[index.row()].title();
    }

    return QVariant();
}

Qt::ItemFlags PrimaryListModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsSelectable
           | Qt::ItemIsEnabled
           | Qt::ItemIsEditable;
}

bool PrimaryListModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (role == Qt::EditRole) {
        m_items[index.row()].setTitle(value.toString());
        emit dataChanged(index, index, {role});
        return true;
    }
    return false;
}

ListItem &PrimaryListModel::getItem(const QModelIndex &index) {
    return m_items[index.row()];
}

void PrimaryListModel::addItem(const ListItem &item) {
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items.append(item);
    endInsertRows();
}

void PrimaryListModel::removeItem(int row) {
    if (row < 0 || row >= m_items.size()) return;
    beginRemoveRows(QModelIndex(), row, row);
    m_items.removeAt(row);
    endRemoveRows();
}
