#ifndef MUSICSTORAGE_H
#define MUSICSTORAGE_H

#include "download.h"
#include "musicobject.h"
#include <QDir>
#include <QObject>
#include <QQueue>

class MusicStorage : public QObject
{
    Q_OBJECT
public:
    explicit MusicStorage(QObject* parent = nullptr);
    void setMusicDir(const QDir &dir);
    QVector<std::shared_ptr<MusicObject>> downloadMusic(QUrl url, int playlistsAllowed);
    std::shared_ptr<MusicObject> queryMusic(const QString &query);
    QVector<std::shared_ptr<MusicObject> > importMusic(QStringList files);
    const QHash<QString, MusicObject> getSongs();
    void setSongs(const QHash<QString, std::shared_ptr<MusicObject>> &songs);
    QHash<QString, std::shared_ptr<MusicObject>> &getSongsShared();
    void checkedRemoveSong(const QString& hash);
    bool isDirty() const;
    void copyAllSongs(const QDir &newPath);
    void moveAllSongs(const QDir &newPath);
    void uncheckedRemoveAll();

private:
    QHash<QString, std::shared_ptr<MusicObject>> m_songs;
    QStringList m_addedHashes;
    QStringList m_addedFiles;
    bool m_dirty;
    QString m_ffmpegPath;
    bool m_canceled;
    Downloader m_downloader;
    QString resolveHash(const QUrl &url);
    bool prepareSongDir(QDir &songDir, const QString &hashValue);
    void convertQueue(QQueue<QString> &queue, const QDir &savePath);
signals:
    void handled();
    void conversionFinished();
    void progressUpdate(int percent, const QString &name, const QString &size, const QString &speed, const QString &ETA);
};

#endif // MUSICSTORAGE_H
