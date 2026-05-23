#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include "musicobject.h"
#include <QObject>
#include <qdir.h>
#include <qprocess.h>

class Downloader : public QObject
{
    Q_OBJECT
public:
    explicit Downloader(QObject* parent = nullptr);
    void downloadSong(const QUrl &url, const QDir &path, const QString &hash, bool allowPlaylists = false);

private:
    QString m_ytdlpPath;
    void handleSimple(const QString &output, const QDir &songPath, const QString &hash);
    void handleMultiple(const QString &output, const QDir &songPath, const QString &hash);
    std::shared_ptr<MusicObject> json2song(const QString &output, const QDir &songPath, const QString &hash, const QString &postfix = QString());
    QProcess *m_process;

private slots:
    void handleDownloadFinished(const QString &output, const QDir &songPath, const QString &hash);

signals:
    void downloadFinished(QVector<std::shared_ptr<MusicObject>> songs);
    void progressUpdate(int percent, const QString &name, const QString &size, const QString &speed, const QString &ETA);
};

#endif // DOWNLOAD_H
