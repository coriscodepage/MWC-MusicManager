#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <QObject>
#include <qdir.h>

class Downloader
{
public:
    Downloader();
    void downloadSong(const QUrl &url, const QDir &path) const;

private:
    QString m_ytdlpPath;
};

#endif // DOWNLOAD_H
