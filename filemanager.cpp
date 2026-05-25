#include "filemanager.h"
#include "musicitem.h"

void FileManager::copySongsToDrive(const QVector<MusicItem> &songs, const InsertController::Drives drive) {
    const static QString subDirs[] = {"CD1", "CD2", "CD3", "Radio"};
    QDir insertDir = m_gamePath;
    if (!insertDir.cd(subDirs[drive])) {
        qWarning() << QString("[FileManager] Cd to %1 failed").arg(subDirs[drive]);
        return;
    }
    QStringList d = insertDir.entryList();
    QStringList oggFiles = d.filter(".ogg", Qt::CaseInsensitive);
    for (const auto &file : std::as_const(oggFiles)) {
        qDebug() << QString("[FileManager] Removing %1").arg(file);
        if (!insertDir.remove(file))
            qWarning() << QString("[FileManager] Removing %1 failed").arg(file);
    }
    int i = 1;
    for (auto &song : songs) {
        auto path = song.songPath();
        if(!path.isEmpty()) {
            QFileInfo into(path);
            int res = QFile::copy(into.absoluteFilePath(), insertDir.filePath(QString("track%1.ogg").arg(i)));
            if (!res)
                qWarning() << QString("[FileManager] Insert of %1 failed").arg(into.fileName());
            else i++;
        }
    }
}

QByteArray FileManager::loadSaveFile() {
    if (m_appSaveName.isEmpty() || m_appSaveName.isEmpty()) {
        qWarning() << "[FileManager] App Save Path or Save Name not set";
        return {};
    }
    qDebug() << "[FileManager] Load start";
    QFile file(m_appPath.filePath(m_appSaveName));

    if (!file.open(QIODevice::ReadOnly))
        return {};
    return file.readAll();
}

void FileManager::setGamePath(QDir path) {
    m_gamePath = path;
}

void FileManager::setMusicPath(QDir path) {
    m_musicPath = path;
}

void FileManager::setAppPath(QDir path) {
    m_appPath = path;
}

void FileManager::setSaveName(QString name) {
    m_appSaveName = name;
}

const QDir &FileManager::getGamePath() const {
    return m_gamePath;
}
const QDir &FileManager::getMusicPath() const {
    return m_musicPath;
}
const QDir &FileManager::getAppPath() const {
    return m_appPath;
}

const QString &FileManager::getSaveName() const {
    return m_appSaveName;
}