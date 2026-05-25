#include "insertcontroller.h"
#include "filemanager.h"
#include <qdebug.h>

InsertController::InsertController(SelectionState *selectionState, QObject *parent)
    : QObject{parent}, m_selectionState(selectionState)
{
    connect(m_selectionState, &SelectionState::songListChanged, this, [this](const ListItem *data) {
        if (!data) return;
        const auto hash = data->getInsertHash();
        revalidateInsert(hash);
    });
}

void InsertController::revalidateInsert(QString hash) {
    const auto listHash = std::find(std::begin(m_inserted), std::end(m_inserted), hash);


    if (listHash != std::end(m_inserted)) {
        int index = std::distance(m_inserted.begin(), listHash);
        switch (index) {
        case Drives::CD1:
            emit insertedChanged(Drives::CD1, true);
            emit insertedChanged(Drives::CD2, false);
            emit insertedChanged(Drives::CD3, false);
            emit insertedChanged(Drives::RADIO, false);
            break;

        case Drives::CD2:
            emit insertedChanged(Drives::CD1, false);
            emit insertedChanged(Drives::CD2, true);
            emit insertedChanged(Drives::CD3, false);
            emit insertedChanged(Drives::RADIO, false);
            break;

        case Drives::CD3:
            emit insertedChanged(Drives::CD1, false);
            emit insertedChanged(Drives::CD2, false);
            emit insertedChanged(Drives::CD3, true);
            emit insertedChanged(Drives::RADIO, false);
            break;

        case Drives::RADIO:
            emit insertedChanged(Drives::CD1, false);
            emit insertedChanged(Drives::CD2, false);
            emit insertedChanged(Drives::CD3, false);
            emit insertedChanged(Drives::RADIO, true);
            break;

        default:
            qWarning() << QString("[InsertController] Unknown state for insert with hash %1").arg(hash);
            break;
        }
    } else {
        emit insertedChanged(Drives::CD1, false);
        emit insertedChanged(Drives::CD2, false);
        emit insertedChanged(Drives::CD3, false);
        emit insertedChanged(Drives::RADIO, false);
    }
    QVector<QString> ins(std::begin(m_inserted), std::end(m_inserted));
    m_selectionState->setInserted(ins);
}

void InsertController::insert(int index) {
    if (index >= 0 && index < 4) {
        const auto currentList = m_selectionState->currenList();
        if (!currentList)
            return;

        const auto currentHash = currentList->getInsertHash();
        auto updatedInserted = m_inserted;
        const auto listHash = std::find(std::begin(updatedInserted), std::end(updatedInserted), currentHash);
        if (listHash != std::end(updatedInserted)) {
            int i = std::distance(updatedInserted.begin(), listHash);
            if (index == i) return;
            updatedInserted[i] = {};
        }
        updatedInserted[index] = currentHash;
        if (!insertSubdirToGame(currentList->getItems(), index, currentHash))
            return;
        m_inserted = updatedInserted;
        revalidateInsert(currentHash);
        qDebug() << QString("[InsertController] Inserting %1 into Drive number %2.").arg(currentHash).arg(index + 1);
    }
}

bool InsertController::insertSubdirToGame(const QVector<MusicItem> &songs, int type, const QString &insertHash) {
    Q_UNUSED(insertHash)
    const static QString subDirs[] = {"CD1", "CD2", "CD3", "Radio"};
    QDir insertDir = FileManager::getInstance().getGamePath();
    if (!insertDir.cd(subDirs[type])) {
        qWarning() << QString("[GameManager] Cd to %1 failed").arg(subDirs[type]);
        return false;
    }
    bool success = true;
    QStringList d = insertDir.entryList();
    QStringList oggFiles = d.filter(".ogg", Qt::CaseInsensitive);
    for (const auto &file : std::as_const(oggFiles)) {
        qDebug() << QString("[GameManager] Removing %1").arg(file);
        if (!insertDir.remove(file))
        {
            qWarning() << QString("[GameManager] Removing %1 failed").arg(file);
            success = false;
        }
    }
    int i = 1;
    for (auto &song : songs) {
        auto path = song.songPath();
        if(!path.isEmpty()) {
            QFileInfo into(path);
            int res = QFile::copy(into.absoluteFilePath(), insertDir.filePath(QString("track%1.ogg").arg(i)));
            if (!res) {
                qWarning() << QString("[GameManager] Insert of %1 failed").arg(into.fileName());
                success = false;
            }
            else i++;
        }
    }
    return success;
    // m_inserted[type] = insertHash;
}

void InsertController::setInserted(QVector<QString> inserted) {
    if (inserted.count() != 4) return;
    for(auto i = 0; i < 4; i++) {
        m_inserted[i] = inserted[i];
        // revalidateInsert(m_inserted[i]);
    }
    QVector<QString> ins(std::begin(m_inserted), std::end(m_inserted));
    m_selectionState->setInserted(ins);
}

QVector<QString> InsertController::getAllInserted() {
    QVector<QString> ret;
    for(auto i = 0; i < 4; i++) {
        ret.push_back(m_inserted.at(i));
    }
    return ret;
}
