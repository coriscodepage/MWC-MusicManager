#include "listitem.h"


ListItem::ListItem(const QString &title, bool type, const QString &hash) : m_title(title), m_type(type), m_insertHash(hash) {
    if (m_insertHash.isEmpty())
        m_insertHash = QString("%1%2").arg(qHash(this)).arg(QDateTime::currentMSecsSinceEpoch());
}

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
    return &m_items.at(index);
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

void ListItem::setType(bool type) {
    m_type = type;
}

const QString &ListItem::getInsertHash() const {
    return m_insertHash;
}

QDataStream &operator<<(QDataStream &out, const ListItem &item) {
    out << item.title();
    out << item.type();
    out << item.getItems();
    out << item.getInsertHash();
    return out;
}

QDataStream &operator>>(QDataStream &in, ListItem &item) {

    QString title;
    bool type;
    QVector<MusicItem> items;
    QString insertHash;
    in >> title;
    in >> type;
    in >> items;
    in >> insertHash;
    ListItem temp_item(title, type, insertHash);
    temp_item.setItems(items);
    item = temp_item;
    return in;
}

