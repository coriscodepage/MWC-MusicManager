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
    QVector<std::shared_ptr<MusicObject> > importMusic(QWidget *parent);
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
    bool m_dirty;
    QString resolveHash(const QUrl &url);
    bool prepareSongDir(QDir &songDir, const QString &hashValue);
    void convertQueue(QQueue<QString> &queue);
signals:
    void handled();
    void conversionFinished();
    void singleConvertFinished(const QString &hash);

};

#endif // MUSICSTORAGE_H
