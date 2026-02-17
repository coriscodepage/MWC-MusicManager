#ifndef LISTITEM_H
#define LISTITEM_H

#include "musicitem.h"
#include <qobject.h>
class ListItem
{
public:
    ListItem(const QString &title, bool type = false);
    const QString &title() const;
    const bool type() const;
    void setTitle(const QString &title);
    void addItem(const MusicItem &item);
    MusicItem *getItem(int row);
    const MusicItem &getItem(int index) const;
    void removeItem(int index);
    int itemCount();

private:
    QString m_title;
    bool m_type;
    QVector<MusicItem> m_items;
};

#endif // LISTITEM_H
