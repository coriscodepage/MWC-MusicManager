#ifndef PRIMARYLISTMODEL_H
#define PRIMARYLISTMODEL_H

#include "listitem.h"
#include "musicstorage.h"
#include "custommodeledit.h"
#include "selectionstate.h"
#include <QStringListModel>
#include <qundostack.h>

class PrimaryListModel : public QAbstractListModel, public CustomModelEdit
{
    Q_OBJECT
public:
    explicit PrimaryListModel(MusicStorage *musicStore, QUndoStack *undoStack, SelectionState *selectionState, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    void addItem(const ListItem &item, int index = -1);
    void removeItem(const QModelIndex &index);
    ListItem &getItem(const QModelIndex &index);

    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList &indexes) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    Qt::DropActions supportedDropActions() const override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;
    void moveInternal(const QVector<QVariant> &movingItems, int sourceRow, int count, int destinationChild) override;

    const QVector<ListItem> &getItems() const;
    QVector<ListItem> &getItems();
    void setItems(const QVector<ListItem> &items);
    void setType(int index, bool type);
    void setField(int field, const QString &value, const QModelIndex &index) override;
    void removeAt(int row) override;
    void insertEmptyAt(int row, const QString &name, bool type) override;
    QString getField(int field, const QModelIndex &index) override;

private:
    QVector<ListItem> m_items;
    mutable QModelIndexList m_draggedIndexes;
    MusicStorage *m_musicStore;
    QUndoStack *m_undoStack;
    SelectionState *m_selectionState;
    void insertAt(const ListItem &item, int index = -1);

signals:
    void typeChanged();
};

#endif // PRIMARYLISTMODEL_H
