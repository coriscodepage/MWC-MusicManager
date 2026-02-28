#include "gamemanager.h"

GameManager::GameManager(const QDir &gamePath): m_gamePath(gamePath) {
    for(int i = 0; i < 4; i++) m_inserted.append(QString());
}

void GameManager::insertSubdirToGame(const QVector<MusicItem> &songs, const Drives &type, const QString &insertHash) {
    const static QString subDirs[] = {"CD1", "CD2", "CD3", "Radio"};
    QDir insertDir = m_gamePath;
    if (!insertDir.cd(subDirs[type])) {
        qWarning() << QString("[GameManager] Cd to %1 failed").arg(subDirs[type]);
        return;
    }
    QStringList d = insertDir.entryList();
    QStringList oggFiles = d.filter(".ogg", Qt::CaseInsensitive);
    for (const auto &file : std::as_const(oggFiles)) {
        qDebug() << QString("[GameManager] Removing %1").arg(file);
        if (!insertDir.remove(file))
            qWarning() << QString("[GameManager] Removing %1 failed").arg(file);
    }
    int i = 1;
    for (auto &song : songs) {
        auto path = song.songPath();
        if(!path.isEmpty()) {
            QFileInfo into(path);
            int res = QFile::copy(into.absoluteFilePath(), insertDir.filePath(QString("track%1.ogg").arg(i)));
            if (!res)
                qWarning() << QString("[GameManager] Insert of %1 failed").arg(into.fileName());
            else i++;
        }
    }
    m_inserted[type] = insertHash;
}

const QString &GameManager::getInsertedHash(const Drives &drive) {
    return m_inserted[drive];
}

void GameManager::setInserted(QVector<QString> &list) {
    if (list.isEmpty()) return;
    m_inserted = list;
}

const QVector<QString> &GameManager::getAllInserted() {
    return m_inserted;
}
