#include "gamemanager.h"

GameManager::GameManager(const QDir &gamePath): m_gamePath(gamePath) {}

void GameManager::insertSubdirToGame(const QVector<MusicItem> &songs, const QString &subDir) {
    QDir insertDir = m_gamePath;
    if (!insertDir.cd(subDir)) {
        qWarning() << QString("[GameManager] Cd to %1 failed").arg(subDir);
        return;
    }
    QStringList d = insertDir.entryList();
    QStringList oggFiles = d.filter(".ogg", Qt::CaseInsensitive);
    for (const auto &file : oggFiles) {
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
}
