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
    QVector<std::shared_ptr<MusicObject>> downloadMusic(QWidget *parent);
    std::shared_ptr<MusicObject> queryMusic(const QString &query);
    QVector<std::shared_ptr<MusicObject> > importMusic(QWidget *parent, QStringList files = {});
    const QHash<QString, MusicObject> getSongs();
    void setSongs(const QHash<QString, std::shared_ptr<MusicObject>> &songs);
    QHash<QString, std::shared_ptr<MusicObject>> &getSongsShared();
    void checkedRemoveSong(const QString& hash);
    const QDir &getMusicDir() const;
    bool isDirty() const;
    void copyAllSongs(const QDir &newPath);
    void moveAllSongs(const QDir &newPath);
    void uncheckedRemoveAll();

private:
    QDir m_musicDir;
    Downloader m_downloader;
    QHash<QString, std::shared_ptr<MusicObject>> m_songs;
    QStringList m_addedHashes;
    QStringList m_addedFiles;
    bool m_dirty;
    QString m_ffmpegPath;
    bool m_canceled;
    QString resolveHash(const QUrl &url);
    bool prepareSongDir(QDir &songDir, const QString &hashValue);
    void convertQueue(QQueue<QString> &queue, const QDir &savePath, QWidget *parent);
signals:
    void handled();
    void conversionFinished();

};

#endif // MUSICSTORAGE_H
