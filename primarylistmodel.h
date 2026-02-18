#ifndef PRIMARYLISTMODEL_H
#define PRIMARYLISTMODEL_H

#include "listitem.h"
#include "musicstorage.h"
#include <QStringListModel>

class PrimaryListModel : public QAbstractListModel
{
public:
    explicit PrimaryListModel(QObject *parent = nullptr, MusicStorage *musicStore = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    void addItem(const ListItem &item);
    void removeItem(const QModelIndex &index);
    ListItem &getItem(const QModelIndex &index);

    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList &indexes) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    Qt::DropActions supportedDropActions() const override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    const QVector<ListItem> &getItems() const;
    QVector<ListItem> &getItems();
    void setItems(const QVector<ListItem> &items);

private:
    QVector<ListItem> m_items;
    mutable QModelIndexList m_draggedIndexes;
    MusicStorage *m_musicStore;
    void insertAt(const ListItem &item, int index = -1);
};

#endif // PRIMARYLISTMODEL_H
