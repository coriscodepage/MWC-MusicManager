#include "musicobject.h"

MusicObject::MusicObject(const QString &title, int duration, const QString &artist, const QDir &fullPath): m_title(title), m_duration(duration),
    m_artist(artist), m_fullPath(fullPath), m_hasThumbnail(true)
{
    QStringList d = m_fullPath.entryList();
    QStringList oggFiles = d.filter(".ogg", Qt::CaseInsensitive);
    QStringList webpFiles = d.filter(".webp", Qt::CaseInsensitive);

    if (!oggFiles.isEmpty())
        m_songPath = m_fullPath.filePath(d.filter(".ogg", Qt::CaseInsensitive).constFirst());
    else {
        qWarning() << QString("[MusicObject] Can't find song in directory: %1").arg(m_fullPath.path());
        m_valid = false;
        return;
    }
    if (!webpFiles.isEmpty())
        m_thumbnailPath = m_fullPath.filePath(d.filter(".webp", Qt::CaseInsensitive).constFirst());
    else {
        qWarning() << QString("[MusicObject] Can't find thumbnail in directory: %1").arg(m_fullPath.path()); // INFO: The lack of a thumbnail does not cause the app to crash and burn, no early return.
        m_hasThumbnail = false;
    }
    m_valid = true;
    qDebug() << QString("[MusicObject] Created song with title: %1, duration %2:%3, artist: %4, path: %5").arg(title).arg(duration/60).arg(duration%60).arg(artist, fullPath.path());
    qDebug() << QString("[MusicObject] Song path: %1, Thumbnail path %2").arg(m_songPath.path(), m_thumbnailPath.path());
}

bool MusicObject::isValid() {
    return m_valid;
}

bool MusicObject::hasThumbnail() {
    return m_hasThumbnail;
}

const QString &MusicObject::title() const {
    return m_title;
}
int MusicObject::duration() const {
    return m_duration;
}

const QString &MusicObject::artist() const {
    return m_artist;
}

QDir MusicObject::thumbnailPath() const {
    if (m_hasThumbnail) return m_thumbnailPath;
    return QDir();
}

const QDir &MusicObject::songPath() const {
    return m_songPath;
}
