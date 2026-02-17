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

void ListItem::addItem(const MusicItem &item) {
    m_items.append(item);
}

MusicItem *ListItem::getItem(int row) {
    return &m_items[row];
}
const MusicItem &ListItem::getItem(int index) const {
    return m_items[index];
}
void ListItem::removeItem(int index) {
    m_items.remove(index);
}

int ListItem::itemCount() {
    return m_items.size();
}
