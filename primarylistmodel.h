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
    QStringList mimeTypes() const override;
    QMimeData* mimeData(const QModelIndexList &indexes) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;
    Qt::DropActions supportedDropActions() const override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;
    void moveInternal(const QVector<QVariant> &movingItems, int sourceRow, int count, int destinationChild) override;
    void beginMacro(const QString &name) override;
    void endMacro() override;
    void setField(int field, const QString &value, const QModelIndex &index) override;
    void removeAt(int row) override;
    void insertEmptyAt(int row, const QString &name, bool type) override;
    QString getField(int field, const QModelIndex &index) const override;
    const QVariant getSelf(QModelIndexList indexes) const override;
    const QVector<QVariant> getDependent() const override;
    void restoreEntry(const QVariant &selfData, const QVector<QVariant> &childData, QModelIndexList indexes) override;

    void addItem(const ListItem &item, int index = -1);
    void removeItem(const QModelIndex &index);
    ListItem &getItem(const QModelIndex &index);
    const QVector<ListItem> &getItems() const;
    QVector<ListItem> &getItems();
    void setItems(const QVector<ListItem> &items);
    void setType(int index, bool type);
    void revalidate(int row);

private:
    QVector<ListItem> m_items;
    MusicStorage *m_musicStore;
    QUndoStack *m_undoStack;
    SelectionState *m_selectionState;
    void insertAt(const ListItem &item, int index = -1);

signals:
    void typeChanged();
};

#endif // PRIMARYLISTMODEL_H
