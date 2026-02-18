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
    std::shared_ptr<MusicObject> downloadMusic(QWidget *parent);
    std::shared_ptr<MusicObject> queryMusic(const QString &query);
    void importMusic(const QDir &path);
    const QHash<QString, MusicObject> getSongs() const;
    void setSongs(const QHash<QString, std::shared_ptr<MusicObject>> &songs);
    void checkedRemoveSong(const QString& hash);

private:
    QDir m_musicDir;
    Downloader m_downloader;
    QHash<QString, std::shared_ptr<MusicObject>> m_songs;

signals:
    void handled();

};

#endif // MUSICSTORAGE_H
