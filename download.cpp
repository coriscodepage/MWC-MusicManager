#include "download.h"
#include <qcontainerfwd.h>
#include <qdebug.h>
#include <QProcess>
#include <qurl.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProgressDialog>
#include <qurlquery.h>

Downloader::Downloader(QObject* parent) : QObject(parent) {
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

void Downloader::downloadSong(const QUrl &url, const QDir &path, const QString &hash, QWidget *parent) {
    QProcess *process = new QProcess(this);
    QStringList arguments;
    QString ytdlpPath = QString(path.absolutePath() % "/%1").arg("%(title)s.%(ext)s");
    qDebug() << QString("[Downloader] Downloading to: %1").arg(ytdlpPath);
    arguments << "-x" << "--write-thumbnail" << "--audio-format" << "vorbis" << "--no-playlist" << "--print-json" << url.toDisplayString() << "-o" << ytdlpPath;

    QProgressDialog *progress = new QProgressDialog("Downloading...", "Abort Download", 0, 100, parent);
    progress->setWindowModality(Qt::ApplicationModal);
    progress->setMinimumDuration(0);
    progress->setValue(50);
    progress->show();

    connect(process, &QProcess::finished, this,
        [this, process, path, progress, hash](int exitCode, QProcess::ExitStatus status) {

            if (exitCode == 0) {
                qDebug() << "[Downloader] Download success";
                QString output = process->readAllStandardOutput();
                progress->setValue(100);
                handleDownloadFinished(output, path, hash);
            } else {
                qWarning() << "[Downloader] Download failed";
                qDebug() << "[Downloader] stderr: " << process->readAllStandardError();
                progress->setValue(100);
                emit downloadFinished(nullptr);
            }

            process->deleteLater();
            progress->deleteLater();
        },
            Qt::SingleShotConnection
    );

    connect(progress, &QProgressDialog::canceled, this, [this, process, progress]() {
        if (process)
            process->kill();
        progress->setValue(100);
    },
            Qt::SingleShotConnection
    );

    process->start(m_ytdlpPath, arguments);

    if (!process->waitForStarted()) {
        qWarning() << "[Downloader] Failed to start yt-dlp";
        return;
    }

}

void Downloader::handleDownloadFinished(const QString &output, const QDir &songPath, const QString &hash) {
    QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8());
    QString title = doc.object().value("title").toString();
    int duration = doc.object().value("duration").toInt();
    QString artist = doc.object().value("uploader").toString();
    QString hashActual = hash;
    QDir checkPath = songPath;
    if (hash == "search") {
        QString sanitizedUrl = doc.object().value("webpage_url").toString().remove("https://").remove("http://").remove("www.");
        QUrlQuery query(QUrl(sanitizedUrl).query());
        if(query.hasQueryItem("v")) {
            hashActual = QString("yt%1").arg(query.queryItemValue("v"));
            qDebug() << "[Downloader] Found v in searched url: " + hashActual;
        }
    }
    QDir storagePath = songPath;
    storagePath.cdUp();
    QDir entryPath = storagePath.relativeFilePath(hashActual);
    auto song = std::make_shared<MusicObject>(title, duration, artist, storagePath, entryPath, hashActual, checkPath);
    emit downloadFinished(song);
}
