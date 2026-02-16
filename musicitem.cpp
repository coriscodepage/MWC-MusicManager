#include "musicitem.h"

MusicItem::MusicItem(const QString &title): m_title(title) {}

const QString &MusicItem::title() const {
    return m_title;
}

void MusicItem::setTitle(const QString &title) {
    m_title = title;
}
