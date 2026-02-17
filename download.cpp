#include "download.h"
#include <qcontainerfwd.h>
#include <qdebug.h>
#include <QProcess>
#include <qurl.h>


Downloader::Downloader() {
    #ifdef Q_OS_WIN
        m_ytdlpPath = "yt-dlp.exe";
    #elif defined(Q_OS_MAC)
        m_ytdlpPath = "yt-dlp";
    #elif defined(Q_OS_LINUX)
        m_ytdlpPath = "yt-dlp";
    #else
        m_ytdlpPath = "yt-dlp";
    #endif
}

void Downloader::downloadSong(const QUrl &url, const QDir &path) const {
    QProcess process;
    QStringList arguments;
    QString ytdlpPath = QString(path.absolutePath() % "/%1").arg("%(title)s.%(ext)s");
    qDebug() << QString("Downloading to: %1").arg(ytdlpPath);
    arguments << "-x" << "--write-thumbnail" << "--audio-format" << "vorbis" << "--no-playlist" << "--print"  << "%(title)s\n%(uploader)s\n%(duration)s" << url.toDisplayString() << "-o" << ytdlpPath;

    process.start(m_ytdlpPath, arguments);

    if (!process.waitForStarted()) {
        qWarning() << "Failed to start yt-dlp";
        return;
    }

    process.waitForFinished(-1); // FIXME: Infinite wait.
    qDebug() << process.readAllStandardOutput();
    qDebug() << process.readAllStandardError();
}
