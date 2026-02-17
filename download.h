#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include "musicobject.h"
#include <QObject>
#include <qdir.h>

class Downloader : public QObject
{
    Q_OBJECT
public:
    explicit Downloader(QObject* parent = nullptr);
    void downloadSong(const QUrl &url, const QDir &path, QWidget *parent);

private:
    QString m_ytdlpPath;

private slots:
    void handleDownloadFinished(const QString &output, const QDir &songPath);

signals:
    void downloadFinished(std::shared_ptr<MusicObject> song);
};

#endif // DOWNLOAD_H
