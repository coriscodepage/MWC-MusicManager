#ifndef SECONDARYLISTMODEL_H
#define SECONDARYLISTMODEL_H

#include "listitem.h"
#include "musicitem.h"
#include <QAbstractListModel>

class SecondaryListModel : public QAbstractListModel
{
public:
    explicit SecondaryListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    void addItem(const MusicItem &item);
    void removeItem(const QModelIndex &index);
    ListItem *getItem();
    void setSource(ListItem *item);

private:
    ListItem *m_item = nullptr;
};

#endif // SECONDARYLISTMODEL_H
