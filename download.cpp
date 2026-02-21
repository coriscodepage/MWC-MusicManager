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

void Downloader::downloadSong(const QUrl &url, const QDir &path, const QString &hash, QWidget *parent, bool allowPlaylists) {
    QProcess *process = new QProcess(this);
    QStringList arguments;
    QString ytdlpPath = QString(path.absolutePath() % "/%1").arg("%(title)s.%(ext)s");
    qDebug() << QString("[Downloader] Downloading to: %1").arg(ytdlpPath);
    arguments << "-x" << "--write-thumbnail" << "--audio-format" << "vorbis";
    if (!allowPlaylists)
        arguments << "--no-playlist";
    arguments << "--print-json" << url.toDisplayString() << "-o" << ytdlpPath;

    QProgressDialog *progress = new QProgressDialog(tr("Downloading..."), tr("Abort Download"), 0, 100, parent);
    progress->setWindowModality(Qt::ApplicationModal);
    progress->setMinimumDuration(0);
    progress->setValue(50);
    progress->show();

    connect(process, &QProcess::finished, this,
        [this, process, path, progress, hash](int exitCode, QProcess::ExitStatus status) {

            QString output = process->readAllStandardOutput();
            if (exitCode == 0 || output.size() > 0) { // FIXME: Just pure heuristics. Yt-dlp will return 1 as the exit code if some playlist items are unavailable but it will still download the rest
                qDebug() << "[Downloader] Download success";
                progress->setValue(100);
                handleDownloadFinished(output, path, hash);
            } else {
                qWarning() << "[Downloader] Download failed";
                qDebug() << "[Downloader] stderr: " << process->readAllStandardError();
                progress->setValue(100);
                emit downloadFinished({});
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
    if (output.trimmed().split("\n").count() > 1)
        handleMultiple(output, songPath, hash);
    else
        handleSimple(output, songPath, hash);
}

void Downloader::handleSimple(const QString &output, const QDir &songPath, const QString &hash) {
    auto song = json2song(output, songPath, hash);
    emit downloadFinished({song});
}

void Downloader::handleMultiple(const QString &output, const QDir &songPath, const QString &hash) {
    QString ownedOutput = output;
    QTextStream stream(&ownedOutput);
    QVector<QString> lines;
    while(!stream.atEnd()) {
        lines << stream.readLine();
    }

    int i = 0;
    QVector<std::shared_ptr<MusicObject>> songs;
    for (auto &line : lines) {
        auto song = json2song(line, songPath, "search", i == 0 ? "plist" : QString()); // FIXME: Not verbose enough about the intent
        songs.append(song);
        i++;
    }
    emit downloadFinished(songs);
}

std::shared_ptr<MusicObject> Downloader::json2song(const QString &output, const QDir &songPath, const QString &hash, const QString &postfix) {
    QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8());
    QString title = doc.object().value("title").toString();
    int duration = doc.object().value("duration").toInt();
    QString artist = doc.object().value("uploader").toString();
    QString url = doc.object().value("webpage_url").toString();
    QString hashActual = hash + postfix;
    QDir checkPath = songPath;
    if (hash == "search") {
        QString sanitizedUrl = url.remove("https://").remove("http://").remove("www."); // FIXME: This is not really exhaustive
        QUrlQuery query(QUrl(sanitizedUrl).query());
        if(query.hasQueryItem("v")) {
            hashActual = QString("yt%1").arg(query.queryItemValue("v")) + postfix;
            qDebug() << "[Downloader] Found v in searched url: " + hashActual;
        }
    }
    QDir storagePath = songPath;
    storagePath.cdUp();
    QDir entryPath = storagePath.relativeFilePath(hashActual);
    auto song = std::make_shared<MusicObject>(title, duration, artist, storagePath, entryPath, hashActual, checkPath, url);
    return song;
}
