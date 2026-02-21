#include "musicobject.h"

MusicObject::MusicObject(const QString &title, int duration, const QString &artist, const QDir &storagePath, const QDir &entryPath, const QString &hash, const QDir &checkPath, const QString &url): m_title(title), m_duration(duration),
    m_artist(artist), m_storagePath(storagePath), m_entryPath(entryPath), m_hash(hash), m_hasThumbnail(true), m_url(url), m_makedForDeletion(false)
{
    QDir actualCheckPath = m_storagePath.absoluteFilePath(entryPath.path());
    if (checkPath != QDir())
        actualCheckPath = checkPath;
    QStringList d = checkPath.entryList();
    QStringList oggFiles = d.filter(".ogg", Qt::CaseInsensitive);
    QStringList imageFiles = d.filter(".webp", Qt::CaseInsensitive) + d.filter(".jpg", Qt::CaseInsensitive) + d.filter(".png", Qt::CaseInsensitive); // FIXME: JANK. USE MIME TYPE.

    if (!oggFiles.isEmpty())
        m_songName = QFileInfo(oggFiles.filter(title).constFirst());
    else {
        qWarning() << QString("[MusicObject] Can't find song in directory: %1").arg(actualCheckPath.path());
        m_valid = false;
        return;
    }
    if (!imageFiles.isEmpty())
        m_thumbnailName = QFileInfo(imageFiles.filter(title).constFirst());
    else {
        qWarning() << QString("[MusicObject] Can't find thumbnail in directory: %1").arg(actualCheckPath.path()); // INFO: The lack of a thumbnail does not cause the app to crash and burn, no early return.
        m_hasThumbnail = false;
        m_thumbnailName = QFileInfo();
    }
    m_valid = true;
    qDebug() << QString("[MusicObject] Created song with title: %1, duration %2:%3, artist: %4, path: %5").arg(title).arg(duration/60).arg(duration%60).arg(artist, storagePath.path());
    qDebug() << QString("[MusicObject] Song name: %1, Thumbnail name %2").arg(m_songName.fileName(), m_thumbnailName.fileName());
}

MusicObject::MusicObject(const QString &title, int duration, const QString &artist, const QDir &entryPath, const QString &hash, bool isValid,
    bool hasThumbnail, const QFileInfo &thumbnailName, const QFileInfo &songName, const QString &url) : m_title(title), m_duration(duration), m_artist(artist),
    m_entryPath(entryPath), m_hash(hash), m_valid(isValid), m_hasThumbnail(hasThumbnail), m_thumbnailName(thumbnailName), m_songName(songName), m_url(url), m_makedForDeletion(false) {}

void MusicObject::setStoragePath(const QDir &path) {
    m_storagePath = path;
}

const QDir &MusicObject::storagePath() const {
    return m_storagePath;
}

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

QString MusicObject::thumbnailPath() const {
    QDir retPath = m_storagePath;
    retPath.cd(m_entryPath.path());
    return retPath.absoluteFilePath(m_thumbnailName.fileName());
}

QString MusicObject::songPath() const {
    QDir retPath = m_storagePath;
    retPath.cd(m_entryPath.path());
    return retPath.absoluteFilePath(m_songName.fileName());
}

const QString &MusicObject::getHash() const {
    return m_hash;
}

const QDir &MusicObject::entryPath() const {
    return m_entryPath;
}

void MusicObject::deleteFromDisk() {
    qDebug() << QString("[MusicObject] Deleting: %1").arg(m_title);
    QDir deletePath = m_storagePath;
    if(!deletePath.cd(m_entryPath.path())) {
        qWarning() << QString("[MusicObject] Entering %1 failed. Bailing").arg(m_entryPath.path());
        return;
    }
    if (!deletePath.removeRecursively())
        qWarning() << QString("[MusicObject] Deleting: %1 failed").arg(m_title);
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

const QFileInfo &MusicObject::songName() const {
    return m_songName;
}

const QFileInfo &MusicObject::thumbnailName() const {
    return m_thumbnailName;
}

QDataStream &operator<<(QDataStream &out, const MusicObject &item) {
    out << item.title();
    out << item.duration();
    out << item.artist();
    out << item.entryPath().path();
    out << item.getHash();
    out << item.isValid();
    out << item.hasThumbnail();
    out << item.m_thumbnailName.fileName();
    out << item.m_songName.fileName();
    out << item.m_url;
    return out;
}

QDataStream &operator>>(QDataStream &in, MusicObject &item) {
    QString title;
    int duration;
    QString artist;
    QString entryPath;
    QString hash;
    bool isValid;
    bool hasThumbnail;
    QString thumbnailName;
    QString songName;
    QString url;

    in >> title >> duration >> artist >> entryPath >> hash >> isValid >> hasThumbnail >> thumbnailName >> songName >> url;
    item = MusicObject(title, duration, artist, QDir(entryPath), hash, isValid, hasThumbnail, QFileInfo(thumbnailName), QFileInfo(songName), url);
    return in;
}
