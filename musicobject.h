#ifndef MUSICOBJECT_H
#define MUSICOBJECT_H

#include <qdir.h>
#include <qobject.h>
class MusicObject
{
public:
    MusicObject(const QString &title, int duration, const QString &artist, const QDir &fullPath);
    bool isValid();
    bool hasThumbnail();
    const QString &title() const;
    int duration() const;
    const QString &artist() const;
    QDir thumbnailPath() const; // INFO: By value cause a dangling reference was annoying me.
    const QDir &songPath() const;

private:
    QString m_title;
    int m_duration;
    QString m_artist;
    QDir m_fullPath;
    QDir m_songPath;
    QDir m_thumbnailPath;
    bool m_valid;
    bool m_hasThumbnail;
};

#endif // MUSICOBJECT_H
