#ifndef SECONDARYLISTMODEL_H
#define SECONDARYLISTMODEL_H

#include "listitem.h"
#include "musicitem.h"
#include "musicstorage.h"
#include <QAbstractListModel>

class SecondaryListModel : public QAbstractListModel
{
public:
    explicit SecondaryListModel(QObject *parent = nullptr, MusicStorage *musicStore = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList &indexes) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    Qt::DropActions supportedDropActions() const override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    const QModelIndexList &copiedIndexes() const;
    // void removeItem(const QModelIndex &index);
    ListItem *getItem();
    void setSource(ListItem *item);

private:
    ListItem *m_item = nullptr;
    MusicStorage *m_musicStore;
    mutable QModelIndexList m_draggedIndexes;

    void addItemAt(const MusicItem &item, int row);
};

#endif // SECONDARYLISTMODEL_H
