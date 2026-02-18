#include "musicobject.h"

MusicObject::MusicObject(const QString &title, int duration, const QString &artist, const QDir &fullPath, const QString &hash): m_title(title), m_duration(duration),
    m_artist(artist), m_fullPath(fullPath), m_hash(hash), m_hasThumbnail(true), m_makedForDeletion(false)
{
    QStringList d = m_fullPath.entryList();
    QStringList oggFiles = d.filter(".ogg", Qt::CaseInsensitive);
    QStringList imageFiles = d.filter(".webp", Qt::CaseInsensitive) + d.filter(".jpg", Qt::CaseInsensitive) + d.filter(".png", Qt::CaseInsensitive); // FIXME: JANK. USE MIME TYPE.

    if (!oggFiles.isEmpty())
        m_songPath = QFileInfo(m_fullPath.filePath(oggFiles.constFirst()));
    else {
        qWarning() << QString("[MusicObject] Can't find song in directory: %1").arg(m_fullPath.path());
        m_valid = false;
        return;
    }
    if (!imageFiles.isEmpty())
        m_thumbnailPath = QFileInfo(m_fullPath.filePath(imageFiles.constFirst()));
    else {
        qWarning() << QString("[MusicObject] Can't find thumbnail in directory: %1").arg(m_fullPath.path()); // INFO: The lack of a thumbnail does not cause the app to crash and burn, no early return.
        m_hasThumbnail = false;
    }
    m_valid = true;
    qDebug() << QString("[MusicObject] Created song with title: %1, duration %2:%3, artist: %4, path: %5").arg(title).arg(duration/60).arg(duration%60).arg(artist, fullPath.path());
    qDebug() << QString("[MusicObject] Song path: %1, Thumbnail path %2").arg(m_songPath.filePath(), m_thumbnailPath.filePath());
}

MusicObject::MusicObject(const QString &title, int duration, const QString &artist, const QDir &fullPath, const QString &hash, bool isValid,
    bool hasThumbnail, const QFileInfo &thumbnailPath, const QFileInfo &songPath) : m_title(title), m_duration(duration), m_artist(artist),
    m_fullPath(fullPath), m_hash(hash), m_valid(isValid), m_hasThumbnail(hasThumbnail), m_thumbnailPath(thumbnailPath), m_songPath(songPath), m_makedForDeletion(false) {}


bool MusicObject::isValid() const {
    return m_valid;
}

bool MusicObject::hasThumbnail() const {
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

QFileInfo MusicObject::thumbnailPath() const {
    if (m_hasThumbnail) return m_thumbnailPath;
    return QFileInfo();
}

const QFileInfo &MusicObject::songPath() const {
    return m_songPath;
}

const QString &MusicObject::getHash() const {
    return m_hash;
}

const QDir &MusicObject::fullPath() const {
    return m_fullPath;
}

void MusicObject::deleteFromDisk() {
    qDebug() << "[MusicObject] Delete not implemented";
}

void MusicObject::markForDeletion() {
    m_makedForDeletion = true;
}

void MusicObject::unmarkForDeletion() {
    m_makedForDeletion = false;
}

bool MusicObject::isForDeletion() {
    return m_makedForDeletion;
}

QDataStream &operator<<(QDataStream &out, const MusicObject &item) {
    out << item.title();
    out << item.duration();
    out << item.artist();
    out << item.fullPath().absolutePath();
    out << item.getHash();
    out << item.isValid();
    out << item.hasThumbnail();
    out << item.thumbnailPath().absoluteFilePath();
    out << item.songPath().absoluteFilePath();
    return out;
}

QDataStream &operator>>(QDataStream &in, MusicObject &item) {
    QString title;
    int duration;
    QString artist;
    QString fullPath;
    QString hash;
    bool isValid;
    bool hasThumbnail;
    QString thumbnailPath;
    QString songPath;

    in >> title >> duration >> artist >> fullPath >> hash >> isValid >> hasThumbnail >> thumbnailPath >> songPath;
    item = MusicObject(title, duration, artist, QDir(fullPath), hash, isValid, hasThumbnail, QFileInfo(thumbnailPath), QFileInfo(songPath));
    return in;
}
