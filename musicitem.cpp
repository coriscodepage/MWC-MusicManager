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
    QDir path = m_song->thumbnailPath();
    if (path.isEmpty() || !path.exists()) return QPixmap(path.absolutePath());
    QPixmap pixmap(path.absolutePath());
    return pixmap;
}

void MusicItem::setTitle(const QString &title) {
    m_title = title;
}

void MusicItem::setSong(std::shared_ptr<MusicObject> song) {
    m_song = song;
    m_title = "";
}
