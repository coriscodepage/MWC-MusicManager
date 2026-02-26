#include "musicstorage.h"
#include "downloaddialog.h"
#include <QUrlQuery>
#include <qeventloop.h>
#include <QFileDialog>
#include <qprocess.h>
#include <qprogressdialog.h>
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QStandardPaths>

MusicStorage::MusicStorage(QObject* parent) : QObject(parent), m_downloader(Downloader(this)), m_dirty(false) {
    #ifdef Q_OS_WIN
        m_ffmpegPath = "ffmpeg.exe";
    #elif defined(Q_OS_MAC)
        m_ffmpegPath = "./ffmpeg";
    #elif defined(Q_OS_LINUX)
        m_ffmpegPath = "./ffmpeg";
    #else
        m_ffmpegPath = "./ffmpeg";
    #endif

    QString ffmpegExe = QStandardPaths::findExecutable("ffmpeg");
    if (!ffmpegExe.isEmpty())
        m_ffmpegPath = ffmpegExe;
}

void MusicStorage::setMusicDir(const QDir &dir) {
    if (!dir.exists()) {
        qWarning() << QString("[MusicStorage] Invalid dir of %1 No dir set.").arg(dir.absolutePath());
        return;
    }
    m_musicDir = dir;
    qDebug() << QString("[MusicStorage] Music storage path: %1").arg(m_musicDir.absolutePath());
}

QString MusicStorage::resolveHash(const QUrl &url)
{
    QUrlQuery query(url.query());
    if (query.hasQueryItem("v")) {
        QString hash = QString("yt%1").arg(query.queryItemValue("v"));
        qDebug() << "[MusicStorage] Found v in url:" << hash;
        return hash;
    }
    QString sanitized = url.toDisplayString().remove("https://").remove("http://").remove("www."); // FIXME: Jank but at least prefix agnostic
    QString hash = QString("url%1").arg(qHash(sanitized));
    qDebug() << "[MusicStorage] Hashing significant url:" << hash;
    return hash;
}

bool MusicStorage::prepareSongDir(QDir &songDir, const QString &hashValue)
{
    if (m_musicDir.exists(hashValue)) {
        qWarning() << "[MusicStorage] Song directory already exists, removing:" << hashValue;
        QDir existing = m_musicDir;
        existing.cd(hashValue);
        existing.removeRecursively();
    }
    if (!m_musicDir.mkpath(hashValue)) {
        qWarning() << "[MusicStorage] Couldn't create song directory:" << m_musicDir.absolutePath() + hashValue;
        return false;
    }
    songDir = m_musicDir;
    songDir.cd(hashValue);
    return true;
}

/// Open dialog. Get the url, check if youtube like and make a hash value for the folder name.
QVector<std::shared_ptr<MusicObject>> MusicStorage::downloadMusic(QWidget *parent) {
    auto dialog = DownloadDialog();
    int res = dialog.exec();
    if (res != QDialog::Accepted) return {};

    auto url = dialog.url();
    int playlistsAllowed = dialog.isPlaylistsAllowed();
    QString hashValue = resolveHash(url);

    if (auto result = queryMusic(hashValue)) {
        qDebug() << "[MusicStorage] Already downloaded";
        m_dirty = true;
        return {result};
    }
    QDir songDir;
    prepareSongDir(songDir, hashValue);

    QEventLoop loop; // INFO: Sync wait, non blocking.
    QObject::connect(this, &MusicStorage::handled, &loop, &QEventLoop::quit, Qt::SingleShotConnection);

    if (url.isValid() && (url.scheme() == "http" || url.scheme() == "https") && !url.host().isEmpty())
        m_downloader.downloadSong(url, songDir, hashValue, parent, playlistsAllowed);
    else {
        qDebug() << "[MusicStorage] Malformed url. Assuming yt search";
        m_downloader.downloadSong("ytsearch1:" + url.toDisplayString(), songDir, "search", parent);
    }

    connect(&m_downloader, &Downloader::downloadFinished, this, [this, songDir](QVector<std::shared_ptr<MusicObject>> objects) {
        bool cleanUpdir = true;
        bool workDone = false;
        for (auto &object : objects) {
            if (!object->isValid()) {
                qDebug() << QString("[MusicStorage] Removing invalid object with title %1").arg(object->title());
                QDir sPath = object->storagePath();
                if(sPath.cd(object->entryPath().path())) {
                    if(sPath.removeRecursively())
                        continue;
                }
                qWarning() << QString("[MusicStorage] Removal of invalid object with title %1 failed").arg(object->title());
                continue;
            }

            if (songDir != object->storagePath().absoluteFilePath(object->getHash())) {
                auto storagePath = songDir;
                auto songName = object->songName();
                auto thumbnailName = object->thumbnailName();
                if(!m_musicDir.mkpath(m_musicDir.absoluteFilePath(object->getHash())))
                    qWarning() << "[MusicStorage] Path creation failed";
                else {
                    QDir destPath(m_musicDir.absoluteFilePath(object->getHash()));

                    const QString srcSong  = storagePath.absoluteFilePath(songName.fileName());
                    const QString destSong = destPath.absoluteFilePath(songName.fileName());

                    if (!QFile::copy(srcSong, destSong))
                        qWarning() << QString("[MusicStorage] Failed to copy song: %1 to %2").arg(srcSong, destSong);
                    else
                        QFile::remove(srcSong);

                    if (!thumbnailName.fileName().isEmpty()) {
                        const QString srcThumb  = storagePath.absoluteFilePath(thumbnailName.fileName());
                        const QString destThumb = destPath.absoluteFilePath(thumbnailName.fileName());

                        if (!QFile::copy(srcThumb, destThumb))
                            qWarning() << QString("[MusicStorage] Failed to copy thumbnail: %1 to %2").arg(srcThumb, destThumb);
                        else
                            QFile::remove(srcThumb);
                    }
                }
            } else {
                cleanUpdir = false;
            }

            if (!m_songs.contains(object->getHash())) {
                m_songs.insert(object->getHash(), object);
                workDone = true;
                qDebug() << QString("[MusicStorage] Insering key: %1 with title: %2").arg(object->getHash(), object->title());
            } else {
                qDebug() << QString("[MusicStorage] Key: %1 with title: %2 already exists. Updating").arg(object->getHash(), object->title());
                m_songs.value(object->getHash())->setStoragePath(object->storagePath()); // TODO: Update more stuff. Dont want to redo the object because that splits the pointers
            }

            m_addedHashes << object->getHash();
        }
        if (cleanUpdir || !workDone) {
            qDebug() << "[MusicStorage] Cleanup dir called";
            QDir cleanupDir = songDir;
            cleanupDir.removeRecursively();
        }
        emit handled();
    }, Qt::SingleShotConnection);
    loop.exec();

    QVector<std::shared_ptr<MusicObject>> returnList;
    for (const auto &hash : std::as_const(m_addedHashes)) {
        if (auto result = queryMusic(hash))
            returnList.append(result);
        else {
            qWarning() << QString("[MusicStorage] Hash %1 added but not present. Silently failing").arg(hash);
        }
    }
    m_addedHashes.clear();
    return returnList;
}

QVector<std::shared_ptr<MusicObject>> MusicStorage::importMusic(QWidget *parent) {
    // for (auto &p : m_songs) {
    //     qDebug() << QString("[MusicStorage] Song with title of %1 has a use count of: %2").arg(p->title()).arg(p.use_count());
    // }
    QStringList files = QFileDialog::getOpenFileNames(
        parent,
        tr("Open Audio Files"),
        QDir::homePath(),
        tr(
            "All Audio Files (*.mp3 *.ogg *.oga *.opus *.flac *.wav *.aiff *.aif *.aac *.m4a *.mp4 *.wma *.wv *.ape *.mpc *.spx *.tta *.dsf *.dff *.ac3 *.dts);;"
            "MP3 (*.mp3);;"
            "OGG Vorbis (*.ogg *.oga);;"
            "Opus (*.opus);;"
            "FLAC (*.flac);;"
            "WAV (*.wav *.aiff *.aif);;"
            "AAC / M4A (*.aac *.m4a *.mp4);;"
            "WMA (*.wma);;"
            "TrueAudio (*.tta);;"
            "DSD (*.dsf *.dff);;"
            "AC3 / DTS (*.ac3 *.dts);;"
            "All Files (*)"
            )
        );
    qDebug() << QString("[MusicStorage] Importing %1 files").arg(files.count());
    QQueue<QString> queue;
    for (const auto &f : std::as_const(files))
        queue.enqueue(f);

    QString tmpFolder = QString("tmp%1").arg(QDateTime::currentSecsSinceEpoch());
    if (!m_musicDir.mkpath(tmpFolder)) {
        qWarning() << "[MusicStorage] Creating temporaty directory for conversion failed. Bailing";
        return {};
    }
    m_addedFiles.clear();
    convertQueue(queue, m_musicDir.absoluteFilePath(tmpFolder), parent);

    auto oldDir = QDir(m_musicDir.absoluteFilePath(tmpFolder));
    if (m_addedFiles.isEmpty()) {
        oldDir.removeRecursively();
        return {};
    }

    QVector<std::shared_ptr<MusicObject>> returnValues;
    QMediaPlayer player(this);
    QEventLoop loop;

    auto conn = connect(&player, &QMediaPlayer::mediaStatusChanged, &loop, [&](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::LoadedMedia || status == QMediaPlayer::InvalidMedia)
            loop.quit();
    });

    for(auto &file : m_addedFiles) {
        auto info = QFileInfo(file);
        auto hash = QString("local%1").arg(qHash(info.baseName()));
        auto newDir = QDir(m_musicDir.absoluteFilePath(hash));

        player.setSource(QUrl::fromLocalFile(oldDir.absoluteFilePath(info.fileName())));

        auto status = player.mediaStatus();
        if (status != QMediaPlayer::LoadedMedia && status != QMediaPlayer::InvalidMedia)
            loop.exec(); // blocks this function but not the app event loop

        std::shared_ptr<MusicObject> obj;
        if (player.mediaStatus() == QMediaPlayer::LoadedMedia) {
            const auto meta = player.metaData();
            auto title    = meta.value(QMediaMetaData::Title).toString();
            auto artist   = meta.value(QMediaMetaData::ContributingArtist).toString();
            auto duration = player.duration();
            obj = std::make_shared<MusicObject>(title.isEmpty() ? info.baseName() : title, duration, artist, m_musicDir, hash, hash, oldDir);
        } else
            obj = std::make_shared<MusicObject>(info.baseName(), 0, "", m_musicDir, hash, hash, oldDir);

        if (obj->isValid()) {
            if(!m_musicDir.mkpath(newDir.absolutePath())) {
                qWarning() << "[MusicStorage] Path creation failed";
                continue;
            }
            const QString srcSong  = oldDir.absoluteFilePath(info.fileName());
            const QString destSong = newDir.absoluteFilePath(info.fileName());

            if (!QFile::copy(srcSong, destSong))
                qWarning() << QString("[MusicStorage] Failed to copy song: %1 to %2").arg(srcSong, destSong);
            else
                QFile::remove(srcSong);

            returnValues.append(obj);
        } else
            obj->deleteFromDisk();
        if (!m_songs.contains(obj->getHash())) {
            m_songs.insert(obj->getHash(), obj);
            qDebug() << QString("[MusicStorage] Insering key: %1 with title: %2").arg(obj->getHash(), obj->title());
        } else {
            qDebug() << QString("[MusicStorage] Key: %1 with title: %2 already exists. Updating").arg(obj->getHash(), obj->title());
            m_songs.value(obj->getHash())->setStoragePath(obj->storagePath()); // TODO: Update more stuff. Dont want to redo the object because that splits the pointers
        }
    }
    oldDir.removeRecursively();
    m_addedFiles.clear();
    disconnect(conn);
    return returnValues;
}

void MusicStorage::convertQueue(QQueue<QString> &queue, const QDir &savePath, QWidget *parent) {
    m_canceled = false;
    QProcess *process = new QProcess(this);
    QProgressDialog *progress = new QProgressDialog(tr("Converting..."), tr("Abort Conversion"), 0, 100, parent); // TODO: Parent
    progress->setWindowModality(Qt::ApplicationModal);
    progress->setMinimumDuration(0);
    progress->setValue(50);
    progress->show();

    connect(progress, &QProgressDialog::canceled, this, [this, process, progress]() {
        if (process)
            process->kill();
        m_canceled = true;
        progress->setValue(100);
    }, Qt::SingleShotConnection);

    QEventLoop loop; // INFO: Sync wait, non blocking.
    connect(this, &MusicStorage::conversionFinished, &loop, &QEventLoop::quit);
    for (auto &queueItem : queue) {
        if (m_canceled) break;
        QString filename = QFileInfo(queueItem).baseName() % ".ogg";
        QStringList arguments;
        arguments << "-i" << QFileInfo(queueItem).absoluteFilePath() << "-c:a" << "libvorbis" << "-q:a" << "4" << "-vn" << savePath.absoluteFilePath(filename);

        connect(process, &QProcess::finished, this,
                [this, process, filename](int exitCode, QProcess::ExitStatus status) {

                    if (exitCode == 0) {
                        qDebug() << "[MusicStorage] Conversion success";
                        m_addedFiles.append(filename);
                        emit conversionFinished();
                    } else {
                        qWarning() << "[MusicStorage] Conversion failed";
                        qDebug() << "[MusicStorage] stderr: " << process->readAllStandardError();
                        emit conversionFinished();
                    }
                }, Qt::SingleShotConnection);
        process->start(m_ffmpegPath, arguments);

        if (!process->waitForStarted()) {
            qWarning() << "[Downloader] Failed to start yt-dlp";
            return;
        }
        loop.exec();
    }
    disconnect(this, &MusicStorage::conversionFinished, &loop, &QEventLoop::quit);
    if(m_canceled) m_addedFiles.clear();
    progress->setValue(100);
    process->deleteLater();
    progress->deleteLater();
    m_canceled = false;
}

std::shared_ptr<MusicObject> MusicStorage::queryMusic(const QString &query) {
    if (m_songs.contains(query)) {
        if (m_songs.value(query)->isForDeletion())
            m_songs.value(query)->unmarkForDeletion();
        return m_songs.value(query);
    }
    return nullptr;
}

const QHash<QString, MusicObject> MusicStorage::getSongs() {
    QHash<QString, MusicObject> temp_map;
    for (auto it = m_songs.constBegin(); it != m_songs.constEnd(); it++) {
        auto key = it.key();
        auto value = *(it.value());
        qDebug() << QString("[MusicStorage] Preparing for save key %1, value %2").arg(key, it.value()->title());
        if (!value.isForDeletion())
            temp_map.insert(key, value);
        else {
            qDebug() << QString("[MusicStorage] Skipping key %1, value %2 as marked for deletion").arg(key, it.value()->title());
            it.value()->deleteFromDisk();
            m_songs.erase(it);
        }
    }
    m_dirty = false;
    return temp_map;
}

QHash<QString, std::shared_ptr<MusicObject>> &MusicStorage::getSongsShared() {
    return m_songs;
}

void MusicStorage::setSongs(const QHash<QString, std::shared_ptr<MusicObject>> &songs) {
    m_songs = songs;
}

bool MusicStorage::isDirty() const {
    return m_dirty;
}

const QDir &MusicStorage::getMusicDir() const {
    return m_musicDir;
}

void MusicStorage::checkedRemoveSong(const QString& hash) {
    auto it = m_songs.find(hash);
    if (it == m_songs.end())
        return;
    qDebug() << QString("[MusicStorage] Delete for title %1 with ref_count of %2 requested").arg(it.value()->title()).arg(it.value().use_count());
    if (it.value().use_count() == 1) {
        it.value()->markForDeletion();
    }
}

void MusicStorage::copyAllSongs(const QDir &newPath) {
    for(auto &song : m_songs) {
        song->copyToNewStoragePath(newPath);
        song->setStoragePath(newPath);
    }
}

void MusicStorage::moveAllSongs(const QDir &newPath) {
    for(auto &song : m_songs) {
        song->moveToNewStoragePath(newPath);
        song->setStoragePath(newPath);
    }
}

void MusicStorage::uncheckedRemoveAll() {
    for(auto &song : m_songs)
        song->deleteFromDisk();
}
