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

    if (auto result = queryMusic(hashValue)) {
        qDebug() << "[MusicStorage] Already downloaded";
        return result;
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
    QObject::connect(this, &MusicStorage::handled, &loop, &QEventLoop::quit, Qt::SingleShotConnection);

    m_downloader.downloadSong(url, songDir, hashValue, parent);
    connect(&m_downloader, &Downloader::downloadFinished, this, [this, hashValue](std::shared_ptr<MusicObject> object) {
        qDebug() << QString("[MusicStorage] Insering key: %1 with addres: %2").arg(hashValue).arg((size_t)object.get());
        if (object != nullptr) {
            if (object->isValid())
                m_songs.insert(hashValue, object);
        }
        emit handled();
    }, Qt::SingleShotConnection);
    loop.exec();
    if (auto result = queryMusic(hashValue))
        return result;
    songDir.removeRecursively();
    return nullptr;
}
void MusicStorage::importMusic(const QDir &path) {
    for (auto &p : m_songs) {
        qDebug() << QString("[MusicStorage] Songo with title of %1 has a use count of: %2").arg(p->title()).arg(p.use_count());
    }
}

std::shared_ptr<MusicObject> MusicStorage::queryMusic(const QString &query) {
    if (m_songs.contains(query)) {
        if (m_songs.value(query)->isForDeletion())
            m_songs.value(query)->unmarkForDeletion();
        return m_songs.value(query);
    }
    return nullptr;
}

const QHash<QString, MusicObject> MusicStorage::getSongs() const {
    QHash<QString, MusicObject> temp_map;
    for (auto it = m_songs.constBegin(); it != m_songs.constEnd(); it++) {
        auto key = it.key();
        auto value = *(it.value());
        qDebug() << QString("[MusicStorage] Preparing for save key %1, value %2").arg(key, it.value()->title());
        temp_map.insert(key, value);
    }
    return temp_map;
}

void MusicStorage::setSongs(const QHash<QString, std::shared_ptr<MusicObject>> &songs) {
    m_songs = songs;
}

void MusicStorage::checkedRemoveSong(const QString& hash) {
    auto it = m_songs.find(hash);
    if (it == m_songs.end())
        return;
    qDebug() << QString("[MusicStorage] Delete for title %1 with ref_count of %2 requested").arg(it.value()->title()).arg(it.value().use_count());
    if (it.value().use_count() == 1) {
        it.value()->markForDeletion();
        m_songs.erase(it);
    }
}

