#ifndef SECONDARYLISTMODEL_H
#define SECONDARYLISTMODEL_H

#include "listitem.h"
#include "musicitem.h"
#include "musicstorage.h"
#include "textfieldedit.h"
#include <QAbstractListModel>
#include <qundostack.h>

class SecondaryListModel : public QAbstractListModel, public TextFieldEdit
{
public:
    explicit SecondaryListModel(QObject *parent = nullptr, MusicStorage *musicStore = nullptr, QUndoStack *undoStack = nullptr);

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
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;
    const QModelIndexList &copiedIndexes() const;
    ListItem *getItem();
    void setSource(ListItem *item);
    void moveInternal(const QVector<MusicItem> &movingItems, int sourceRow, int count, int destinationChild);
    QString getTitle(int index) const;
    QString getArtist(int index) const;
    QString getHash(int index) const;
    QPixmap getPixmap(int index) const;
    QString getSongPath(int index) const;
    const QVector<MusicItem> &getSongs() const;
    void setHash(const QString &hash, int index);
    bool insertRowInternal(int row, MusicItem &item);
    void setField(int field, const QString &value, const QModelIndex &index) override;
    QString getField(int field, const QModelIndex &index) override;

private:
    ListItem *m_item = nullptr;
    MusicStorage *m_musicStore;
    QUndoStack *m_undoStack;
    mutable QModelIndexList m_draggedIndexes;

    void addItemAt(const MusicItem &item, int row);
};

#endif // SECONDARYLISTMODEL_H
