#include "librarycontroller.h"
#include "filemanager.h"
#include "insertcommand.h"
#include "listitem.h"
#include <qdebug.h>
#include <qsettings.h>

LibraryController::LibraryController(PrimaryListModel *listModel, SecondaryListModel *songModel, SelectionState *selectionState, MusicStorage *musicStore, QUndoStack *undoStack, QObject *parent)
    : QObject{parent}, m_listModel(listModel), m_songModel(songModel), m_selectionState(selectionState), m_musicStore(musicStore), m_undoStack(undoStack)
{
    prepareDirectories();
}

void LibraryController::loadAppData()
{
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

    m_insertController->setInserted(insertedList);
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

void LibraryController::importDirectory(int type)
{
    QDir directory;
    bool listType = true;
    static QStringList names{tr("CD1"), tr("CD2"), tr("CD3"), tr("Radio"), tr("Custom")};
    emit getDirectory(DirType::GAME);
    switch (type)
    {
    case DefinedDirs::CD1:
        // directory = m_gameDir;
        if (!directory.cd("CD1"))
        {
            qWarning() << "[MainWindow] Can't cd to requested directory";
            return;
        }
        break;
    case DefinedDirs::CD2:
        // directory = m_gameDir;
        if (!directory.cd("CD2"))
        {
            qWarning() << "[MainWindow] Can't cd to requested directory";
            return;
        }
        break;
    case DefinedDirs::CD3:
        // directory = m_gameDir;
        if (!directory.cd("CD3"))
        {
            qWarning() << "[MainWindow] Can't cd to requested directory";
            return;
        }
        break;
    case DefinedDirs::RADIO:
        // directory = m_gameDir;
        if (!directory.cd("Radio"))
        {
            qWarning() << "[MainWindow] Can't cd to requested directory";
            return;
        }
        listType = false;
        break;
    case DefinedDirs::OTHER:
        // directory = getDir(tr("Select directory for import"), QDir::homePath());;
        names[4] = directory.dirName().isEmpty() ? names.at(4) : directory.dirName();
        break;
    }

    QUndoCommand *macro = new QUndoCommand(tr("Import directory"));
    new InsertCommand(m_listModel, names.at(type), listType, m_listModel->rowCount(), macro);
    new InsertCommand(m_songModel, tr("Imported song"), listType,  0, macro);
    m_undoStack->push(macro);

    auto info = directory.entryInfoList();
    QStringList list;
    if (info.empty())
    {
        // showWarningBox(tr("Nothing to import. Directory is empty"));
        return;
    }

    // for (const auto &i : std::as_const(info)) {
    //     list << i.absoluteFilePath();
    // }

    // const auto songList = m_musicStore->importMusic(this, list);
    // if (songList.isEmpty()) {
    //     qWarning() << "[MainWindow] Import failed or cancelled";
    //     return;
    // }

    // const auto indexes = ui->listItemView->selectionModel()->selectedIndexes();
    // if (indexes.isEmpty()) {
    //     qWarning() << "[MainWindow] No selection to apply the import to";
    //     return;
    // }

    // const QModelIndex selection = indexes.constFirst();
    // const QModelIndex index = m_songModel->index(selection.row());
    // auto *listItem = m_songModel->getItem();

    // listItem->getItem(index.row())->setHash(songList.constFirst()->getHash());
    // listItem->getItem(index.row())->setSong(songList.constFirst());

    // for (int i = 1; i < songList.count(); ++i) {
    //     auto item = MusicItem(songList.at(i)->title(), songList.at(i));
    //     m_songModel->insertRowInternal(index.row() + i, item);
    // }

    // setWindowModified(true);
    // m_stickyModified = true;
    // ui->listItemView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
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
                FileManager::getInstance().setGamePath(dir);
            }
            else
            {
                emit directoryInvalid(type, FileManager::getInstance().getGamePath().absolutePath().isEmpty());
            }
        }
        else
        {
            emit directoryInvalid(type, FileManager::getInstance().getGamePath().absolutePath().isEmpty());
        }
        break;
    case APP:
        if (!path.isEmpty())
        {
            QFileInfo info(path);
            if (info.isFile())
            {
                settings.setValue("saveLocation", QVariant::fromValue(info.absoluteFilePath()));
                FileManager::getInstance().setAppPath(info.dir());
                FileManager::getInstance().setSaveName(info.fileName());
            }
            else if (info.isDir())
            {
                const QDir appDir(info.absoluteFilePath());
                settings.setValue("saveLocation", QVariant::fromValue(appDir.absoluteFilePath("save.msc")));
                FileManager::getInstance().setAppPath(appDir);
                FileManager::getInstance().setSaveName("save.msc");
            }
            else
            {
                emit directoryInvalid(type, FileManager::getInstance().getAppPath().absolutePath().isEmpty());
            }
        }
        else
        {
            emit directoryInvalid(type, FileManager::getInstance().getAppPath().absolutePath().isEmpty());
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
            emit directoryInvalid(type, FileManager::getInstance().getMusicPath().absolutePath().isEmpty());
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
            FileManager::getInstance().setGamePath(QDir(path));
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
            FileManager::getInstance().setMusicPath(QDir(path));
            settings.setValue("musicdir", QVariant::fromValue(path));
        }
        else
        {
            emit getDirectory(DirType::MUSIC);
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
            FileManager::getInstance().setAppPath(info.dir());
            FileManager::getInstance().setSaveName(info.fileName());
        }
    }
    else
    {
        if (settings.contains("gamedir"))
        {
            auto path = settings.value("gamedir").toString();
            settings.setValue("saveLocation", QVariant::fromValue(QDir(path).absoluteFilePath("save.msc")));
        }
    }

    // setUiEnabled(false);
    // setGameDir(false);
    // setSaveFile(false);
    // setMusicDir(false);
    // if (m_gameDir.exists())
    //     setUiEnabled(true);
}

// bool MainWindow::setMusicDir(bool exp) {
//     QSettings settings("Kori", "Music Manager");
//     QString musicDirName = "music";
//     QDir musicDir = m_gameDir;

//     if (exp) {
//         QString path = getDir(tr("Select music directory"), m_gameDir.exists() ? m_gameDir.absolutePath() : QDir::homePath());
//         if (!path.isEmpty() && QDir(path).exists()) {
//             settings.setValue("musicdir", QVariant::fromValue(path));
//             musicDir = QDir(path);
//         } else {
//             showWarningBox(tr("Please select a valid directory."));
//             return false;
//         }
//     } else {
//         if (settings.contains("musicdir")) {
//             auto path = settings.value("musicdir").toString();
//             if (!path.isEmpty() && QDir(path).exists()) {
//                 musicDir = QDir(path);
//                 settings.setValue("musicdir", QVariant::fromValue(path));
//             } else {
//                 showWarningBox(tr("Music directory invalid. Defaulting to game directory. If that is incorrect change it in the settings."));
//                 settings.setValue("musicdir", QVariant::fromValue(m_gameDir.absolutePath()));
//             }
//         }
//     }

//     if (!musicDir.exists(musicDirName)) {
//         if (!musicDir.mkpath(musicDirName)) {
//             qWarning() << "[MainWindow] Failed to create music directory";
//             return false;
//         }
//     }
//     if (!musicDir.cd(musicDirName)) {
//         qWarning() << "[MainWindow] Failed to enter music subdirectory";
//         return false;
//     }
//     m_musicStore->setMusicDir(musicDir);
//     return exp;
// }

// bool MainWindow::setGameDir(bool exp) {
//     QSettings settings("Kori", "Music Manager");
//     if (exp) {
//         QDir dir;
//         QString path;
//         do {
//             path = getDir(tr("Select game directory"), m_gameDir.exists() ? m_gameDir.absolutePath() : QDir::homePath());
//             dir = QDir(path);
//             if (dir.exists() && !path.isEmpty()) {
//                 if(dir.entryList().contains("mysummercar.exe") || dir.entryList().contains("mywintercar.exe")) { // INFO: Check if game is really here
//                     QString saveVal = dir.absolutePath();
//                     settings.setValue("gamedir", QVariant::fromValue(saveVal));
//                     m_gameDir = dir;
//                     setMusicDir(false);
//                     return true;
//                 }
//             }
//             showWarningBox(tr("Directory does not contain mysummercar.exe or mywintercar.exe, please select a valid directory."));
//         } while(!dir.exists() || path.isEmpty());
//     } else {
//         if (settings.contains("gamedir")) {
//             auto path = settings.value("gamedir").toString();
//             if (!path.isEmpty() && QDir(path).exists()) {
//                 m_gameDir = QDir(path);
//                 return false;
//             }
//         }
//         setGameDir(true);
//     }
//     return false;
// }

// bool MainWindow::setSaveFile(bool exp, bool open) {
//     QSettings settings("Kori", "Music Manager");

//     if (exp) {
//         if (open) {
//             QString saveFilePath = QFileDialog::getOpenFileName(this,tr("Open File"), m_gameDir.exists() ? m_gameDir.absolutePath() : QDir::homePath() , tr("MusicManager save (*.msc)"));
//             QFileInfo info(saveFilePath);
//             if (!info.isFile()) return false;
//             m_appSave = info;
//             settings.setValue("saveLocation", QVariant::fromValue(m_appSave.absoluteFilePath()));
//             return true;
//         } else {
//             QString saveFilePath = QFileDialog::getSaveFileName(this, tr("Save File"), m_appSave.exists() ? m_appSave.absolutePath() : m_gameDir.exists() ? m_gameDir.absolutePath() : QDir::homePath(), tr("MusicManager save (*.msc)"));
//             m_appSave = QFileInfo(saveFilePath);
//             settings.setValue("saveLocation", QVariant::fromValue(m_appSave.absoluteFilePath()));
//             return true;
//         }
//     } else {
//         if (settings.contains("saveLocation")) {
//             QString saveFilePath = settings.value("saveLocation").toString();
//             QFileInfo info(saveFilePath);
//             if (!info.isFile()) {
//                 setSaveFile(true);
//                 return true;
//             } else {
//                 m_appSave = info;
//                 return false;
//             }
//         } else {
//             if (m_gameDir.exists())
//                 settings.setValue("saveLocation", QVariant::fromValue(m_gameDir.absoluteFilePath("save.msc")));
//         }
//     }
//     return false;
// }

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