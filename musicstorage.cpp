#include "musicstorage.h"
#include "downloaddialog.h"
#include <QUrlQuery>
#include <qeventloop.h>

MusicStorage::MusicStorage(QObject* parent) : QObject(parent), m_downloader(Downloader(this)) {}

void MusicStorage::setMusicDir(const QDir &dir) {
    if (!dir.exists()) {
        qWarning() << QString("[MusicStorage] Invalid dir of %1 No dir set.").arg(dir.absolutePath());
        return;
    }
    qDebug() << QString("[MusicStorage] Music storage path: %1").arg(m_musicDir.absolutePath());
    m_musicDir = dir;
}

/// Open dialog. Get the url, check if youtube like and make a hash value for the folder name.
std::shared_ptr<MusicObject> MusicStorage::downloadMusic(QWidget *parent) {
    auto dialog = DownloadDialog();
    int res = dialog.exec();
    if (res != QDialog::Accepted) return nullptr;

    auto url = dialog.url();
    QUrlQuery query(url.query());
    QString hashValue;
    if(query.hasQueryItem("v")) {
        hashValue = QString("yt%1").arg(query.queryItemValue("v"));
        qDebug() << "[MusicStorage] Found v in url: " + hashValue;
    } else {
        QString sanitizedUrl = url.toDisplayString().remove("https://").remove("http://").remove("www."); // FIXME: Jank but at least prefix agnostic
        auto hashed = qHash(sanitizedUrl);
        hashValue = QString("url%1").arg(hashed);
        qDebug() << "[MusicStorage] Hashing significant url: " + hashValue;
    }

    if (m_songs.contains(hashValue)) {
        qDebug() << "[MusicStorage] Already downloaded";
        return m_songs.value(hashValue);
    }
    QDir songDir = m_musicDir;
    if(!m_musicDir.exists(hashValue)) {
        bool res = m_musicDir.mkpath(hashValue);
        if (res == 0) {
            qWarning() << QString("[MusicStorage] Couldn't create song directory at: /%1").arg(m_musicDir.absolutePath() + hashValue);
            return nullptr;
        }
        songDir.cd(hashValue);
    } else {
        qWarning() << QString("[MusicStorage] Song directory already exists at: /%1").arg(m_musicDir.absolutePath() + hashValue);
        return nullptr;
    }
    QEventLoop loop; // INFO: Sync wait, non blocking.
    QObject::connect(this, &MusicStorage::handled, &loop, &QEventLoop::quit);

    m_downloader.downloadSong(url, songDir, parent);
    connect(&m_downloader, &Downloader::downloadFinished, this, [this, hashValue](std::shared_ptr<MusicObject> object) {
        qDebug() << &object;
        if (object != nullptr) {
            if (object->isValid())
                m_songs.insert(hashValue, object);
        }
        emit handled();
    });
    loop.exec();
    if (m_songs.contains(hashValue))
        return m_songs.value(hashValue);
    songDir.removeRecursively();
    return nullptr;
}
void MusicStorage::importMusic(const QDir &path) {

}
