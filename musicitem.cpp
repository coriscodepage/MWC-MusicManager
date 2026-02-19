#include "musicitem.h"
#include <qpixmap.h>

MusicItem::MusicItem(const QString &title, std::shared_ptr<MusicObject> song): m_title(title), m_song(song), m_hash(QString()) {}

const QString &MusicItem::title() const {
    if (m_title.isEmpty()) {
        if(m_song != nullptr)
            return m_song->title();
    }
    return m_title;
}

void MusicItem::setTitle(const QString &title) {
    m_title = title;
}

void MusicItem::setSong(std::shared_ptr<MusicObject> song) {
    if (song == nullptr) {
        m_song = nullptr;
        return;
    }
    m_song = song;
    m_title = "";
}

QString MusicItem::songPath() const {
    if(m_song == nullptr) return QString();
    return m_song->songPath();
}

const QString &MusicItem::titleInternal() const {
    return m_title;
}

QString MusicItem::pixmapPath() const {
    if(m_song == nullptr) return QString();
    return m_song->thumbnailPath();
}

const QString &MusicItem::getHash() const {
    return m_hash;
}
void MusicItem::setHash(const QString &hash) {
    m_hash = hash;
}

bool MusicItem::hasThumbnail() {
    if(m_song == nullptr) return false;
    return m_song->hasThumbnail();
}

QDataStream &operator<<(QDataStream &out, const MusicItem &item) {
    out << item.titleInternal();
    out << item.getHash();
    return out;
}

QDataStream &operator>>(QDataStream &in, MusicItem &item)
{
    QString title;
    QString hash;
    in >> title;
    in >> hash;
    MusicItem temp_item(title, nullptr);
    temp_item.setHash(hash);
    item = temp_item;
    return in;
}
