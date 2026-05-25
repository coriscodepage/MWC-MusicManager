#ifndef SECONDARYLISTMODEL_H
#define SECONDARYLISTMODEL_H

#include "listitem.h"
#include "musicitem.h"
#include "musicstorage.h"
#include "custommodeledit.h"
#include <QAbstractListModel>
#include <qundostack.h>

class SecondaryListModel : public QAbstractListModel, public CustomModelEdit
{
public:
    explicit SecondaryListModel(MusicStorage *musicStore, QUndoStack *undoStack, QObject *parent = nullptr);

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
    void setField(int field, const QString &value, const QModelIndex &index) override;
    QString getField(int field, const QModelIndex &index) const override;
    const QVariant getSelf(QModelIndexList indexes) const override;
    const QVector<QVariant> getDependent() const override;
    void restoreEntry(const QVariant &selfData, const QVector<QVariant> &childData, QModelIndexList indexes) override;
    void removeAt(int row) override;
    void insertEmptyAt(int row, const QString &name, bool type) override;
    void moveInternal(const QVector<QVariant> &movingItems, int sourceRow, int count, int destinationChild) override;
    void beginMacro(const QString &name) override;
    void endMacro() override;

    const MusicItem* getSong(int row);
    const QModelIndexList &copiedIndexes() const;
    ListItem *getItem();
    void setSource(ListItem *item);
    const QVector<MusicItem> &getSongs() const;
    bool insertRowInternal(int row, MusicItem &item);

public slots:
    void setForceCopy(bool state);

private:
    ListItem *m_list = nullptr;
    MusicStorage *m_musicStore;
    QUndoStack *m_undoStack;
    bool m_forceCopy;

    void addItemAt(const MusicItem &item, int row);

signals:
    void selectionChanged(const QModelIndex &index);

};

#endif // SECONDARYLISTMODEL_H
