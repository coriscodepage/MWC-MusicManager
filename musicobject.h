#ifndef MUSICOBJECT_H
#define MUSICOBJECT_H

#include <qdir.h>
#include <qobject.h>
class MusicObject
{
public:
    MusicObject() = default;
    MusicObject(const QString &title, int duration, const QString &artist, const QDir &fullPath, const QString &hash);
    MusicObject(const QString &title, int duration, const QString &artist, const QDir &fullPath, const QString &hash, bool isValid, bool hasThumbnail, const QFileInfo &thumbnailPath, const QFileInfo &songPath);
    bool isValid() const;
    bool hasThumbnail() const;
    const QString &title() const;
    int duration() const;
    const QString &artist() const;
    QFileInfo thumbnailPath() const; // INFO: By value cause a dangling reference was annoying me.
    const QFileInfo &songPath() const;
    const QDir &fullPath() const;
    const QString &getHash() const;
    void deleteFromDisk();
    void markForDeletion();
    void unmarkForDeletion();
    bool isForDeletion();
    friend QDataStream &operator<<(QDataStream &out, const MusicObject &item);
    friend QDataStream &operator>>(QDataStream &in, MusicObject &item);

private:
    QString m_title;
    int m_duration;
    QString m_artist;
    QDir m_fullPath;
    QFileInfo m_songPath;
    QFileInfo m_thumbnailPath;
    QString m_hash;
    bool m_valid;
    bool m_hasThumbnail;
    bool m_makedForDeletion;
};

Q_DECLARE_METATYPE(MusicObject);


#endif // MUSICOBJECT_H
