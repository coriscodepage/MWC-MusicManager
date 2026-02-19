#ifndef MUSICOBJECT_H
#define MUSICOBJECT_H

#include <qdir.h>
#include <qobject.h>
class MusicObject
{
public:
    MusicObject() = default;
    MusicObject(const QString &title, int duration, const QString &artist, const QDir &storagePath, const QDir &entryPath, const QString &hash, const QDir &checkPath = QDir());
    MusicObject(const QString &title, int duration, const QString &artist, const QDir &entryPath, const QString &hash, bool isValid, bool hasThumbnail, const QFileInfo &thumbnailName, const QFileInfo &songName);
    bool isValid() const;
    bool hasThumbnail() const;
    const QString &title() const;
    int duration() const;
    const QString &artist() const;
    QString thumbnailPath() const;
    const QFileInfo &extracted() const;
    QString songPath() const;
    const QDir &entryPath() const;
    const QString &getHash() const;
    void deleteFromDisk();
    void markForDeletion();
    void unmarkForDeletion();
    bool isForDeletion();
    void setStoragePath(const QDir &path);
    friend QDataStream &operator<<(QDataStream &out, const MusicObject &item);
    friend QDataStream &operator>>(QDataStream &in, MusicObject &item);

private:
    QString m_title;
    int m_duration;
    QString m_artist;
    QDir m_storagePath;
    QDir m_entryPath;
    QFileInfo m_songName;
    QFileInfo m_thumbnailName;
    QString m_hash;
    bool m_valid;
    bool m_hasThumbnail;
    bool m_makedForDeletion;
};

Q_DECLARE_METATYPE(MusicObject);


#endif // MUSICOBJECT_H
