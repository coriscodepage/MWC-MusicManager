#include "musicitem.h"
#include <qpixmap.h>

MusicItem::MusicItem(const QString &title, std::shared_ptr<MusicObject> song): m_title(title), m_song(song) {}

const QString &MusicItem::title() const {
    if (m_title.isEmpty()) {
        if(m_song != nullptr)
            return m_song->title();
    }
    return m_title;
}

QPixmap MusicItem::pixmap() const {
    const QString def = ":/defaults/no-image.webp";
    if(m_song == nullptr) return QPixmap(def);
    if (!m_song->hasThumbnail()) return QPixmap(def);
    QFileInfo path = m_song->thumbnailPath();
    if (!path.isFile() || !path.exists()) return QPixmap(def);
    QPixmap pixmap(path.absoluteFilePath());
    return pixmap;
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

QFileInfo MusicItem::songPath() const {
    if(m_song == nullptr) return QFileInfo();
    return m_song->songPath();
}

const QString &MusicItem::titleInternal() const {
    return m_title;
}

const QString &MusicItem::getHash() const {
    return m_hash;
}
void MusicItem::setHash(const QString &hash) {
    m_hash = hash;
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
