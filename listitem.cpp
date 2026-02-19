#include "listitem.h"


ListItem::ListItem(const QString &title, bool type) : m_title(title), m_type(type) {}

const QString &ListItem::title() const {
    return m_title;
}

const bool ListItem::type() const {
    return m_type;
}

void ListItem::setTitle(const QString &title) {
    m_title = title;
}

void ListItem::addItem(const MusicItem &item, int index) {
    if (index >= 0) {
        m_items.insert(index, item);
    } else
        m_items.append(item);
}

MusicItem *ListItem::getItem(int index) {
    if (index < 0 || index >= m_items.size()) return nullptr;
    return &m_items[index];
}
const MusicItem *ListItem::getItem(int index) const {
    if (index < 0 || index >= m_items.size()) return nullptr;
    return &m_items[index];
}
void ListItem::removeItem(int index) {
    if (index < 0 || index >= m_items.size()) return;
    m_items[index].setSong(nullptr);
    m_items.remove(index);
}

int ListItem::itemCount() {
    return m_items.size();
}

const QVector<MusicItem> &ListItem::getItems() const {
    return m_items;
}

QVector<MusicItem> &ListItem::getItems() {
    return m_items;
}

void ListItem::setItems(const QVector<MusicItem> &items) {
    m_items = items;
}

QDataStream &operator<<(QDataStream &out, const ListItem &item) {
    out << item.title();
    out << item.type();
    out << item.getItems();
    return out;
}

QDataStream &operator>>(QDataStream &in, ListItem &item) {

    QString title;
    bool type;
    QVector<MusicItem> items;
    in >> title;
    in >> type;
    in >> items;
    ListItem temp_item(title, type);
    temp_item.setItems(items);
    item = temp_item;
    return in;
}

