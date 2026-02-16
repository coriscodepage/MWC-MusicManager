#ifndef PRIMARYLISTMODEL_H
#define PRIMARYLISTMODEL_H

#include "listitem.h"
#include <QStringListModel>

class PrimaryListModel : public QAbstractListModel
{
public:
    explicit PrimaryListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    void addItem(const ListItem &item);
    void removeItem(int row);
    ListItem &getItem(const QModelIndex &index);

private:
    QVector<ListItem> m_items;
};

#endif // PRIMARYLISTMODEL_H
