#ifndef LISTITEM_H
#define LISTITEM_H

#include "musicitem.h"
#include <qobject.h>
class ListItem
{
public:
    ListItem() = default;
    ListItem(const QString &title, bool type = false);
    const QString &title() const;
    const bool type() const;
    void setTitle(const QString &title);
    void addItem(const MusicItem &item, int index = -1);
    MusicItem *getItem(int index);
    const MusicItem *getItem(int index) const;
    const QVector<MusicItem> &getItems() const;
    QVector<MusicItem> &getItems();
    void setItems(const QVector<MusicItem> &items);
    void removeItem(int index);
    int itemCount();
    void setType(bool type);

    friend QDataStream &operator<<(QDataStream &out, const ListItem &item);
    friend QDataStream &operator>>(QDataStream &in, ListItem &item);
private:
    QString m_title;
    bool m_type;
    QVector<MusicItem> m_items;
};

Q_DECLARE_METATYPE(ListItem)

#endif // LISTITEM_H
