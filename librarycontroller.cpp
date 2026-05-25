#include "librarycontroller.h"
#include "filemanager.h"
#include "listitem.h"
#include <qdebug.h>
#include <qsettings.h>

LibraryController::LibraryController(PrimaryListModel *listModel, SecondaryListModel *songModel, SelectionState *selectionState, MusicStorage *musicStore, InsertController *insertController, QUndoStack *undoStack, QObject *parent)
    : QObject{parent}, m_listModel(listModel), m_songModel(songModel), m_selectionState(selectionState), m_musicStore(musicStore), m_insertController(insertController), m_undoStack(undoStack)
{}

void LibraryController::loadAppData()
{
    prepareDirectories();
    auto data = FileManager::getInstance().loadSaveFile();
    QDataStream in(&data, QIODevice::ReadOnly);
    QVector<ListItem> primaryList;
    QHash<QString, MusicObject> songsOwned;
    QString musicPath;
    QVector<QString> insertedList;
    in >> primaryList >> songsOwned >> insertedList >> musicPath;
    m_listModel->setItems(std::move(primaryList));
    QHash<QString, std::shared_ptr<MusicObject>> songsShared;

    for (auto it = songsOwned.constBegin(); it != songsOwned.constEnd(); it++)
    {
        auto key = it.key();
        auto value = it.value();
        value.setStoragePath(musicPath);
        qDebug() << QString("[LibraryController] Loading song key: %1, value: %2").arg(key, value.title());
        songsShared.insert(key, std::make_shared<MusicObject>(value));
    }

    m_insertController->setInserted(std::move(insertedList));
    m_musicStore->setSongs(songsShared);

    // if (m_musicStore->getMusicDir() != musicPath)
    // musicMismatch(!musicPath.isEmpty() && QDir(musicPath).exists(), musicPath);

    int secondarySum = 0;
    for (auto &prim : m_listModel->getItems())
    {
        qDebug() << QString("[LibraryController] Primary list has %1 elements").arg(prim.itemCount());
        for (auto &sec : prim.getItems())
        {
            qDebug() << QString("[LibraryController] List %1 title: %2 ").arg(prim.title(), sec.title());
            QString hash = sec.getHash();
            auto songPtr = m_musicStore->queryMusic(hash);
            sec.setSong(songPtr);
            secondarySum++;
        }
    }
    qDebug() << QString("[LibraryController] Loaded %1 primary elements, %2 secondary elements and %3 songs").arg(m_listModel->getItems().length()).arg(secondarySum).arg(songsShared.size());
}

void LibraryController::handleDirecorySelected(DirType type, const QString path)
{
    QSettings settings("Kori", "Music Manager");
    switch (type)
    {

    case GAME:
        if (!path.isEmpty() && QDir(path).exists())
        {
            const auto dir = QDir(path);
            const auto entries = dir.entryList();
            if (entries.contains("mysummercar.exe") || entries.contains("mywintercar.exe"))
            {
                settings.setValue("gamedir", QVariant::fromValue(dir.absolutePath()));
                FileManager::getInstance().setGamePath(path);
            }
            else
            {
                emit directoryInvalid(type, QDir(FileManager::getInstance().getGamePath()).absolutePath().isEmpty());
            }
        }
        else
        {
            emit directoryInvalid(type, QDir(FileManager::getInstance().getGamePath()).absolutePath().isEmpty());
        }
        break;
    case APP:
        if (!path.isEmpty())
        {
            QFileInfo info(path);
            if (info.isFile())
            {
                settings.setValue("saveLocation", QVariant::fromValue(info.absoluteFilePath()));
                FileManager::getInstance().setAppPath(info.dir().absolutePath());
                FileManager::getInstance().setSaveName(info.fileName());
            }
            else if (info.isDir())
            {
                const QDir appDir(info.absoluteFilePath());
                settings.setValue("saveLocation", QVariant::fromValue(appDir.absoluteFilePath("save.msc")));
                FileManager::getInstance().setAppPath(info.absoluteFilePath());
                FileManager::getInstance().setSaveName("save.msc");
            }
            else
            {
                emit directoryInvalid(type, FileManager::getInstance().getAppPath().isEmpty());
            }
        }
        else
        {
            emit directoryInvalid(type, FileManager::getInstance().getAppPath().isEmpty());
        }
        break;
    case MUSIC:
        if (!path.isEmpty() && QDir(path).exists())
        {
            settings.setValue("musicdir", QVariant::fromValue(path));
            FileManager::getInstance().setMusicPath(path);
        }
        else
        {
            emit directoryInvalid(type, FileManager::getInstance().getMusicPath().isEmpty());
        }
        break;
    case CUSTOM:
        break;
    }
}

void LibraryController::prepareDirectories()
{
    QSettings settings("Kori", "Music Manager");
    for (auto &key : settings.allKeys())
    {
        qDebug() << QString("[LibraryController] Setting with key %1 set to %2").arg(key, settings.value(key).toString());
    }

    if (settings.contains("gamedir"))
    {
        auto path = settings.value("gamedir").toString();
        if (!path.isEmpty() && QDir(path).exists())
        {
            FileManager::getInstance().setGamePath(path);
        }
        else
        {
            emit getDirectory(DirType::GAME);
        }
    }
    else
    {
        emit getDirectory(DirType::GAME);
    }

    if (settings.contains("musicdir"))
    {
        auto path = settings.value("musicdir").toString();
        if (!path.isEmpty() && QDir(path).exists())
        {
            FileManager::getInstance().setMusicPath(path);
        }
        else
        {
            emit getDirectory(DirType::MUSIC);
        }
    }
    else if (settings.contains("gamedir"))
    {
        auto path = settings.value("gamedir").toString();
        if (!path.isEmpty() && QDir(path).exists())
        {
            FileManager::getInstance().setMusicPath(path);
        }
    }
    else
    {
        emit getDirectory(DirType::MUSIC);
    }

    if (settings.contains("saveLocation"))
    {
        QString saveFilePath = settings.value("saveLocation").toString();
        QFileInfo info(saveFilePath);
        if (!info.isFile())
        {
            emit getDirectory(DirType::APP);
        }
        else
        {
            FileManager::getInstance().setAppPath(info.dir().absolutePath());
            FileManager::getInstance().setSaveName(info.fileName());
        }
    }
    else if (settings.contains("gamedir"))
    {
            auto path = settings.value("gamedir").toString();
            settings.setValue("saveLocation", QVariant::fromValue(QDir(path).absoluteFilePath("save.msc")));
    } else {
        emit getDirectory(DirType::APP);
    }

    // setUiEnabled(false);
    // setGameDir(false);
    // setSaveFile(false);
    // setMusicDir(false);
    // if (m_gameDir.exists())
    //     setUiEnabled(true);
}

void LibraryController::saveAppData() {
// if (!m_appSave.isFile()) setSaveFile(true, false);
// if (!m_appSave.isFile()) m_appSave = QFileInfo(m_gameDir.absoluteFilePath("save.msc"));
QFileInfo savePath = QFileInfo(QDir(FileManager::getInstance().getAppPath()).absoluteFilePath(FileManager::getInstance().getSaveName()));
qDebug() << "[MainWindow] Save start";
qDebug() << QString("[MainWindow] Saving to: %1").arg(savePath.absoluteFilePath());
QFile file(savePath.absoluteFilePath());
if (!file.open(QIODevice::WriteOnly))
    return;
QDataStream out(&file);
auto primaryItems = m_listModel->getItems();
auto songs = m_musicStore->getSongs();
auto insertedList = m_insertController->getAllInserted();
m_undoStack->clear();
out << primaryItems;
out << songs;
out << insertedList;
out << QDir(FileManager::getInstance().getMusicPath()).absolutePath();
qDebug() << QString("[MainWindow] Saved %1 primary elements and %2 songs").arg(primaryItems.size()).arg(songs.size());
}

// void MainWindow::musicMismatch(bool oldExists, const QDir &oldDir) {
//     auto songsShared = m_musicStore->getSongsShared();
//     QMessageBox msgBox(this);
//     msgBox.setWindowTitle(tr("Music directory mismatch"));
//     msgBox.setText(tr("Saved music directory does not match the current one. Choose how to solve this issue."));
//     QPushButton *setToButton = nullptr;
//     QPushButton *copyButton =   nullptr;
//     QPushButton *moveButton =   nullptr;
//     QPushButton *deleteButton = nullptr;
//     if (oldExists) {
//         deleteButton = msgBox.addButton(tr("Delete old (New session)"), QMessageBox::ActionRole);
//         if (oldDir.exists())
//             setToButton = msgBox.addButton(tr("Set music direcory to the saved one"), QMessageBox::ActionRole);
//         copyButton = msgBox.addButton(tr("Copy old to new"), QMessageBox::ActionRole);
//         moveButton = msgBox.addButton(tr("Move old to new"), QMessageBox::ActionRole);
//     } else {
//         deleteButton = msgBox.addButton(tr("New session"), QMessageBox::ActionRole);
//     }
//     QPushButton *abortButton = msgBox.addButton(QMessageBox::Abort);
//     msgBox.exec();
//     if(msgBox.clickedButton() == abortButton) {
//         exit(EXIT_FAILURE);
//     } else if (copyButton != nullptr && msgBox.clickedButton() == copyButton) {
//         m_musicStore->copyAllSongs(m_musicStore->getMusicDir());
//     } else if (moveButton != nullptr && msgBox.clickedButton() == moveButton) {
//         m_musicStore->moveAllSongs(m_musicStore->getMusicDir());
//     } else if (deleteButton != nullptr && msgBox.clickedButton() == deleteButton) {
//         m_primarymodel->removeRows(0, m_primarymodel->rowCount());
//         m_musicStore->uncheckedRemoveAll();
//         m_musicStore->setSongs({});
//     } else if (setToButton != nullptr && msgBox.clickedButton() == setToButton) {
//         m_musicStore->setMusicDir(oldDir);
//     } else {
//         exit(EXIT_FAILURE);
//     }
//     m_musicStore->setSongs(songsShared);
// }