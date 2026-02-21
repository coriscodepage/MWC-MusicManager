#ifndef MUSICSTORAGE_H
#define MUSICSTORAGE_H

#include "download.h"
#include "musicobject.h"
#include <QDir>
#include <QObject>

class MusicStorage : public QObject
{
    Q_OBJECT
public:
    explicit MusicStorage(QObject* parent = nullptr);
    void setMusicDir(const QDir &dir);
    QVector<std::shared_ptr<MusicObject>> downloadMusic(QWidget *parent);
    std::shared_ptr<MusicObject> queryMusic(const QString &query);
    void importMusic(QWidget *parent);
    const QHash<QString, MusicObject> getSongs();
    void setSongs(const QHash<QString, std::shared_ptr<MusicObject>> &songs);
    void checkedRemoveSong(const QString& hash);
    const QDir &getMusicDir() const;
    bool isDirty() const;

private:
    QDir m_musicDir;
    Downloader m_downloader;
    QHash<QString, std::shared_ptr<MusicObject>> m_songs;
    QStringList m_addedHashes;
    bool m_dirty;
    QString resolveHash(const QUrl &url);
    bool prepareSongDir(QDir &songDir, const QString &hashValue);
signals:
    void handled();

};

#endif // MUSICSTORAGE_H
