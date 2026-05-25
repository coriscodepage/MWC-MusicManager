#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "cutcommand.h"
#include "downloaddialog.h"
#include "insertcontroller.h"
#include "insertcommand.h"
#include "pastecommand.h"
#include "primarylistdelegate.h"
#include "primarylistmodel.h"
#include "removecommand.h"
#include "toggletypecommand.h"
#include "filemanager.h"
#include <QSettings>
#include <QFileDialog>
#include <QAudioOutput>
#include <QCloseEvent>
#include <QClipboard>
#include <qmimedata.h>
#include <QMessageBox>
#include <QImageReader>
#include <QSignalMapper>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug() << "Supported formats:" << QImageReader::supportedImageFormats();

    m_undoStack = new QUndoStack(this);
    m_musicStore = new MusicStorage(this);
    // prepareDirectories();
    // qDebug() << QString("[MainWindow] Game path: %1").arg(m_gameDir.absolutePath());
    m_selectionState = new SelectionState();
    m_primarymodel = new PrimaryListModel(m_musicStore, m_undoStack, m_selectionState, this);
    m_secondarymodel = new SecondaryListModel(m_musicStore, m_undoStack, m_selectionState, this);
    m_mediaPlayer = new MediaPlayer(m_selectionState, this);
    m_selectionController = new SelectionController(m_primarymodel, m_secondarymodel, m_selectionState, this);
    m_insertController = new InsertController(m_selectionState, this);
    m_libraryController = new LibraryController(m_primarymodel, m_secondarymodel, m_selectionState, m_musicStore, m_insertController, m_undoStack, this);
    m_progressBar = new ProgressDialog(this);
    ui->listView->setModel(m_primarymodel);
    ui->listItemView->setModel(m_secondarymodel);
    ui->listView->setItemDelegate(new PrimaryListDelegate());

    connect(m_musicStore, &MusicStorage::progressUpdate, m_progressBar, &ProgressDialog::updateProgress);
    connect(m_progressBar, &ProgressDialog::cancel, m_musicStore, &MusicStorage::cancel);
    connect(m_progressBar, &ProgressDialog::cancel, this, [this](){m_progressBar->accept();});
    connect(ui->listView, &TargetListView::forceCopy, ui->listItemView, &ChildListView::setForceCopy);
    connect(ui->listItemView, &ChildListView::forceCopy, m_secondarymodel, &SecondaryListModel::setForceCopy);

    // connect(ui->actionSetGameDir, &QAction::triggered, this, [this]() {
    //     bool res = setGameDir(true);
    //     if(!m_gameDir.exists()) setUiEnabled(false);
    //     QSettings settings("Kori", "Music Manager");
    //     if (res) {
    //         m_stickyModified = true;
    //     }
    // });

    // connect(ui->actionOpen, &QAction::triggered, this, [this]() {setSaveFile(true);});
    // connect(ui->actionSaveAs, &QAction::triggered, this, [this]() {setSaveFile(true, false);});
    connect(ui->actionClose, &QAction::triggered, this, &QApplication::quit);

    // connect(ui->actionSetMusicDir, &QAction::triggered, this, [this]() {
    //     QDir oldDir = m_musicStore->getMusicDir();
    //     bool res = setMusicDir(true);
    //     if(!m_musicStore->getMusicDir().exists()) setUiEnabled(false);
    //     if(res) {
    //         musicMismatch(true, oldDir);
    //         m_stickyModified = true;
    //     }
    // });

    connect(ui->actionSetDirsDefault, &QAction::triggered, this, [this]()
            {
        QSettings settings("Kori", "Music Manager");
        settings.remove("musicdir");
        showInfoBox(tr("Save and restart to set the locations.")); });

    QSignalMapper *signalMapperImport = new QSignalMapper(this);
    int i = 0;
    for (QAction *action : {ui->actionCD1, ui->actionCD2, ui->actionCD3, ui->actionRadio, ui->actionCustom})
    {
        connect(action, &QAction::triggered, signalMapperImport, qOverload<>(&QSignalMapper::map));
        signalMapperImport->setMapping(action, i++);
    }
    connect(signalMapperImport, &QSignalMapper::mappedInt, this, &MainWindow::importDirectory);

    connect(ui->listView->selectionModel(), &QItemSelectionModel::selectionChanged, m_selectionController, &SelectionController::handlePrimaryListSelectionChanged);
    connect(ui->listItemView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection &selected) {
        QModelIndexList indexes = ui->listItemView->selectionModel()->selectedRows();
        QItemSelection selection;
        for (const auto &index : std::as_const(indexes))
            selection.select(index, index);
        m_selectionController->handleSecondaryListSelectionChanged(selection);
    });

    connect(m_primarymodel, &QAbstractItemModel::dataChanged, this, [this](){updateItemCountLabel(nullptr);});

    connect(m_selectionState, &SelectionState::songListChanged, this, [this](const ListItem *data) {
        bool state = data;
        ui->listItemView->setEnabled(state);
        ui->newItem->setEnabled(state);
        ui->deleteList->setEnabled(state);
        bool listPopulated = data ? data->itemCount() > 0 : false;
        ui->insertGroupBox->setEnabled(listPopulated);
    });
    connect(m_selectionState, &SelectionState::songListChanged, this, &MainWindow::updateItemCountLabel);

    connect(m_selectionState, &SelectionState::songSelected, this, [this](MusicInfo *info, QPixmap pixmap) {
        ui->downloadControls->setEnabled(info);
        pixmap = pixmap.scaled(ui->centralwidget->size()/4, Qt::KeepAspectRatio);
        ui->songArt->setPixmap(pixmap);
        if (!info) {
            ui->songLabel->setText({});
            ui->artistLabel->setText({});
            ui->playSong->setEnabled(false);
            return;
        }
        ui->songLabel->setText(info->title);
        ui->artistLabel->setText(info->artist);
        if (info->valid) {
            ui->playSong->setEnabled(true);
        } else {
            ui->playSong->setEnabled(false);
        }
    });

    // connect(m_secondarymodel, &QAbstractItemModel::rowsRemoved, this, [this]() {
    //     ui->downloadControls->setEnabled(m_secondarymodel->rowCount() > 0);
    //     // updateItemCountLabel();
    // }); // FIXME: MOVE THIS INTO A CONTROLLER WHAT EVEN IS THIS????
    // connect(m_primarymodel, &QAbstractItemModel::rowsRemoved, this, [this]() {
    //     // updateItemCountLabel();
    //     if(m_primarymodel->rowCount() == 0) {
    //         m_secondarymodel->setSource(nullptr);
    //         // handlePrimaryListSelectionChanged(QModelIndex(), QModelIndex());
    //     }
    // }); // FIXME: MOVE THIS INTO A CONTROLLER WHAT EVEN IS THIS????
    connect(m_primarymodel, &QAbstractListModel::rowsRemoved, this, [this](const QModelIndex &parent, int first, int last)
            {
                int rowCount = m_primarymodel->rowCount();
                if (rowCount == 0)
                    return;
                int newRow = qMin(first, rowCount - 1);
                ui->listView->selectionModel()->setCurrentIndex(m_primarymodel->index(newRow, 0, parent), QItemSelectionModel::ClearAndSelect);
                ui->listView->setFocus();
                // if (roles.count() == 0) return; // FIXME: What does this even do?
                // if (roles.constFirst() == Qt::EditRole) setWindowModified(true); // FIXME: What does this even do?
            });

    connect(m_primarymodel, &QAbstractListModel::rowsInserted, this, [this](const QModelIndex &parent, int first, int last)
            {
        ui->listView->selectionModel()->setCurrentIndex(m_primarymodel->index(first, 0 , parent), QItemSelectionModel::ClearAndSelect);
        ui->listView->setFocus();
    });

    connect(m_secondarymodel, &QAbstractListModel::rowsRemoved, this, [this](const QModelIndex &parent, int first, int last)
            {
        updateItemCountLabel(nullptr);
        int rowCount = m_secondarymodel->rowCount();
        if (rowCount == 0) return;
        int newRow = qMin(first, rowCount - 1);
        ui->listItemView->selectionModel()->setCurrentIndex(m_secondarymodel->index(newRow, 0 , parent), QItemSelectionModel::ClearAndSelect);
        ui->listItemView->setFocus();
    });

    connect(m_secondarymodel, &QAbstractListModel::rowsInserted, this, [this](const QModelIndex &parent, int first, int last)
            {
        updateItemCountLabel(nullptr);
        ui->listItemView->selectionModel()->select(m_secondarymodel->index(first, 0 , parent), QItemSelectionModel::ClearAndSelect);
        ui->listItemView->setFocus();
    });

    connect(m_insertController, &InsertController::insertedChanged, this, [this](InsertController::Drives drive, bool inserted)
            {
        switch(drive) {
            case InsertController::CD1:
                ui->insertCD1->setStyleSheet(inserted ? "font-weight: bold;" : "");
            break;
            case InsertController::CD2:
                ui->insertCD2->setStyleSheet(inserted ? "font-weight: bold;" : "");
            break;
            case InsertController::CD3:
                ui->insertCD3->setStyleSheet(inserted ? "font-weight: bold;" : "");
            break;
            case InsertController::RADIO:
                ui->insertRadio->setStyleSheet(inserted ? "font-weight: bold;" : "");
            break;
        }
        m_primarymodel->revalidate(ui->listView->selectionModel()->selectedRows().constFirst().row());
        // m_stickyModified = true;
    });

    QSignalMapper *signalMapperInsert = new QSignalMapper(this);
    i = 0;
    for (QPushButton *button : {ui->insertCD1, ui->insertCD2, ui->insertCD3, ui->insertRadio})
    {
        connect(button, &QPushButton::clicked, signalMapperInsert, qOverload<>(&QSignalMapper::map));
        signalMapperInsert->setMapping(button, i++);
    }
    connect(signalMapperInsert, &QSignalMapper::mappedInt, m_insertController, &InsertController::insert);

    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listView, &QListView::customContextMenuRequested, this, &MainWindow::showListContextMenu);
    ui->listItemView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listItemView, &QListView::customContextMenuRequested, this, &MainWindow::showListItemContextMenu);

    connect(ui->actionSave, &QAction::triggered, this, [this]() {
        m_libraryController->saveAppData();
        // setWindowModified(false);
        // m_stickyModified = false;
    });
    connect(ui->actionCopy, &QAction::triggered, this, &MainWindow::copy);
    connect(ui->actionPaste, &QAction::triggered, this, &MainWindow::paste);
    connect(ui->actionCut, &QAction::triggered, this, &MainWindow::cut);

    connect(ui->stopPlayer, &QPushButton::clicked, m_mediaPlayer, &MediaPlayer::stop);
    connect(ui->pausePlayer, &QPushButton::clicked, m_mediaPlayer, &MediaPlayer::pause);
    connect(ui->playPlayer, &QPushButton::clicked, m_mediaPlayer, &MediaPlayer::play);
    connect(m_mediaPlayer, &MediaPlayer::labelChanged, ui->labelPlayer, &QLabel::setText);
    connect(m_mediaPlayer, &MediaPlayer::stopState, ui->stopPlayer, &QPushButton::setEnabled);
    connect(m_mediaPlayer, &MediaPlayer::pauseState, ui->pausePlayer, &QPushButton::setEnabled);
    connect(m_mediaPlayer, &MediaPlayer::playState, ui->playPlayer, &QPushButton::setEnabled);

    connect(ui->playSong, &QPushButton::clicked, m_mediaPlayer, &MediaPlayer::changeSong);

    connect(m_libraryController, &LibraryController::getDirectory, this, [this](LibraryController::DirType type)
            {
        const QString dir = getDir(type);
        m_libraryController->handleDirecorySelected(type, dir); });

    connect(m_libraryController, &LibraryController::directoryInvalid, this, [this](LibraryController::DirType type, bool retry)
            {
        switch (type) {
        case LibraryController::GAME:
            showWarningBox(tr("Select a valid game directory containing mysummercar.exe or mywintercar.exe."));
            break;
        case LibraryController::APP:
            showWarningBox(tr("Select a valid save file or a directory where save.msc can be stored."));
            break;
        case LibraryController::MUSIC:
            showWarningBox(tr("Select a valid music directory."));
            break;
        case LibraryController::CUSTOM:
            showWarningBox(tr("Select a valid directory."));
            break;
        }

        if (retry) {
            const QString dir = getDir(type);
            m_libraryController->handleDirecorySelected(type, dir);
        } });

    connect(m_undoStack, &QUndoStack::cleanChanged, this, [this](bool clean)
            {
        if (!m_stickyModified)
            setWindowModified(!clean); });

    QAction *action_undo = m_undoStack->createUndoAction(this, tr("Undo"));
    QAction *action_redo = m_undoStack->createRedoAction(this, tr("Redo"));

    action_undo->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
    action_undo->setIcon(QIcon::fromTheme("edit-undo"));
    action_redo->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
    action_redo->setIcon(QIcon::fromTheme("edit-redo"));
    QAction *firstEditAction = ui->menuEdit->actions().constFirst();
    ui->menuEdit->insertAction(firstEditAction, action_undo);
    ui->menuEdit->insertAction(firstEditAction, action_redo);

    ui->listItemView->setDragEnabled(true);
    ui->listItemView->setAcceptDrops(true);
    ui->listItemView->setDropIndicatorShown(true);
    ui->listItemView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->listItemView->setDefaultDropAction(Qt::MoveAction);

    ui->listView->setDragEnabled(true);
    ui->listView->setAcceptDrops(true);
    ui->listView->setDropIndicatorShown(true);
    ui->listView->setDefaultDropAction(Qt::MoveAction);

    // if(m_appSave.isFile())
    //     loadAppData();
    // else if(QFile::exists(m_gameDir.absoluteFilePath("data.msc"))) {
    //     qDebug() << "[MainWindow] Loading save from backup location";
    //     m_appSave = QFileInfo(m_gameDir.absoluteFilePath("data.msc"));
    //     loadAppData();
    // }
    m_libraryController->loadAppData();
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::getDir(LibraryController::DirType type)
{
    QString message;
    QString defaultPath;

    auto pickPath = [](
                        const QDir &primary,
                        const QDir &fallbackGame)
    {
        if (!primary.absolutePath().isEmpty() && primary.exists())
        {
            return primary.absolutePath();
        }
        if (!fallbackGame.absolutePath().isEmpty() && fallbackGame.exists())
        {
            return fallbackGame.absolutePath();
        }
        return QDir::homePath();
    };

    const auto &fileManager = FileManager::getInstance();
    switch (type)
    {
    case LibraryController::GAME:
        message = tr("Select game directory");
        defaultPath = pickPath(fileManager.getGamePath(), QDir());
        break;
    case LibraryController::APP:
        message = tr("Select save file or directory");
        defaultPath = pickPath(fileManager.getAppPath(), fileManager.getGamePath());
        break;
    case LibraryController::MUSIC:
        message = tr("Select music directory");
        defaultPath = pickPath(fileManager.getMusicPath(), fileManager.getGamePath());
        break;
    case LibraryController::CUSTOM:
        message = tr("Select directory");
        defaultPath = pickPath(QDir(), fileManager.getGamePath());
        break;
    }

    QString path = QFileDialog::getExistingDirectory(
        this,
        message,
        defaultPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    return path;
}

void MainWindow::on_addCD_clicked()
{
    InsertCommand *cmd = new InsertCommand(m_primarymodel, tr("New list"), true, m_primarymodel->rowCount());
    m_undoStack->push(cmd);
}

void MainWindow::on_newItem_clicked()
{
    InsertCommand *cmd = new InsertCommand(m_secondarymodel, tr("New song"), true, m_secondarymodel->rowCount());
    m_undoStack->push(cmd);
}

void MainWindow::on_deleteList_clicked()
{
    auto indexes = ui->listView->selectionModel()->selectedRows();

    auto *cmd = new RemoveCommand(m_primarymodel, indexes);

    m_undoStack->push(cmd);
    ui->listView->setFocus();
    if (m_primarymodel->rowCount() > 0)
        ui->listView->selectionModel()->setCurrentIndex(m_primarymodel->index(indexes.constFirst().row() - 1, 0), QItemSelectionModel::ClearAndSelect);
}

void MainWindow::on_downloadItem_clicked()
{
    auto dialog = DownloadDialog();
    int res = dialog.exec();
    if (res != QDialog::Accepted)
        return;

    auto url = dialog.url();
    int playlistsAllowed = dialog.isPlaylistsAllowed();
    m_progressBar->show();
    const auto songList = m_musicStore->downloadMusic(url, playlistsAllowed);
    if (songList.isEmpty())
    {
        qWarning() << "[MainWindow] Download failed or cancelled";
        return;
    }

    const auto indexes = m_selectionState->currentSelection();
    if (indexes.isEmpty())
    {
        qWarning() << "[MainWindow] No selection to apply the download to";
        return;
    }

    const QModelIndex selection = indexes.indexes().constFirst();
    const QModelIndex index = m_secondarymodel->index(selection.row());
    auto *listItem = m_secondarymodel->getItem();

    listItem->getItem(index.row())->setHash(songList.constFirst()->getHash());
    listItem->getItem(index.row())->setSong(songList.constFirst());

    for (int i = 1; i < songList.count(); ++i)
    {
        auto item = MusicItem(songList.at(i)->title(), songList.at(i));
        m_secondarymodel->insertRowInternal(index.row() + i, item);
    }
    ui->listItemView->setFocus();
    if (songList.count()) {
        setWindowModified(true);
        m_secondarymodel->revalidate(index.row());
        m_stickyModified = true;
    }
    m_progressBar->accept();
    // updateItemCountLabel();
    // songUpdated(index);
}

void MainWindow::on_addRadio_clicked()
{
    InsertCommand *cmd = new InsertCommand(m_primarymodel, tr("New list"), false, m_primarymodel->rowCount());
    m_undoStack->push(cmd);
    ui->listView->setFocus();
}

void MainWindow::showInfoBox(const QString &message)
{
    QMessageBox::information(this, tr("Information"), message);
}

void MainWindow::showWarningBox(const QString &message)
{
    QMessageBox::warning(this, tr("Warning"), message);
}

void MainWindow::showErrorBox(const QString &message)
{
    QMessageBox::critical(this, tr("Error"), message);
}

void MainWindow::setUiEnabled(bool enabled)
{
    centralWidget()->setEnabled(enabled);
    menuBar()->setEnabled(enabled);
}

void MainWindow::showListContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->listView->indexAt(pos);
    QModelIndexList list;
    list << index;

    if (!index.isValid())
        return;

    QMenu contextMenu(tr("Context menu"), this);

    QAction action1(tr("Edit"), this);
    QAction action2(tr("Delete"), this);
    QAction action3(tr("Copy"), this);
    QAction action4(tr("Cut"), this);
    QAction action5(tr("Paste"), this);
    QAction action6(tr("Change type"), this);
    contextMenu.addAction(&action1);
    contextMenu.addAction(&action2);
    contextMenu.addAction(&action3);
    contextMenu.addAction(&action4);
    contextMenu.addAction(&action5);
    contextMenu.addAction(&action6);

    auto res = contextMenu.exec(ui->listView->viewport()->mapToGlobal(pos));
    if (res == &action1)
        ui->listView->edit(index);
    else if (res == &action2)
    {
        auto *cmd = new RemoveCommand(m_primarymodel, list);
        m_undoStack->push(cmd);
    }
    else if (res == &action3)
    {
        QMimeData *data = m_primarymodel->mimeData(list);
        QApplication::clipboard()->setMimeData(data);
    }
    else if (res == &action4)
    {
        QMimeData *data = m_secondarymodel->mimeData(list);
        QByteArray binary = data->data(m_secondarymodel->mimeTypes().constFirst());
        CutCommand *cmd = new CutCommand(m_primarymodel, binary, m_secondarymodel->mimeTypes().constFirst(), ui->listView->selectionModel()->selectedRows());
        m_undoStack->push(cmd);
        // int firstRow = list.first().row();
        // int rowCount = list.count();
        // m_secondarymodel->removeRows(firstRow, rowCount);
    }
    else if (res == &action5)
    {
        int firstRow = list.first().row();
        const QMimeData *data = QApplication::clipboard()->mimeData();
        QString format = data->formats().constFirst();
        QByteArray binary = data->data(format);
        PasteCommand *cmd = new PasteCommand(ui->listView->model(), binary, format, firstRow);
        m_undoStack->push(cmd);
    }
    else if (res == &action6)
    {
        auto *cmd = new ToggleTypeCommand(m_primarymodel, index);
        m_undoStack->push(cmd);
    }
}

void MainWindow::showListItemContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->listItemView->indexAt(pos);
    QModelIndexList list;
    list << index;

    if (!index.isValid())
        return;

    QMenu contextMenu(tr("Context menu"), this);

    QAction action1(tr("Edit"), this);
    QAction action2(tr("Delete"), this);
    QAction action3(tr("Copy"), this);
    QAction action4(tr("Cut"), this);
    QAction action5(tr("Paste"), this);
    contextMenu.addAction(&action1);
    contextMenu.addAction(&action2);
    contextMenu.addAction(&action3);
    contextMenu.addAction(&action4);
    contextMenu.addAction(&action5);

    auto res = contextMenu.exec(ui->listItemView->viewport()->mapToGlobal(pos));
    if (res == &action1)
        ui->listItemView->edit(index);
    else if (res == &action2)
    {
    auto *cmd = new RemoveCommand(m_secondarymodel, list);
    m_undoStack->push(cmd);
    }
    else if (res == &action3)
    {
        QMimeData *data = m_secondarymodel->mimeData(list);
        QApplication::clipboard()->setMimeData(data);
    }
    else if (res == &action4)
    {
        QMimeData *data = m_secondarymodel->mimeData(list);
        QByteArray binary = data->data(m_secondarymodel->mimeTypes().constFirst());
        CutCommand *cmd = new CutCommand(m_secondarymodel, binary, m_secondarymodel->mimeTypes().constFirst(), m_selectionState->currentSelection().indexes());
        m_undoStack->push(cmd);
        // int firstRow = list.first().row();
        // int rowCount = list.count();
        // m_secondarymodel->removeRows(firstRow, rowCount);
    }
    else if (res == &action5)
    {
        int firstRow = list.first().row();
        const QMimeData *data = QApplication::clipboard()->mimeData();
        QString format = data->formats().constFirst();
        QByteArray binary = data->data(format);
        PasteCommand *cmd = new PasteCommand(ui->listItemView->model(), binary, format, firstRow);
        m_undoStack->push(cmd);
    }
}

// void MainWindow::songUpdated(const QModelIndex &index) {
//     setWindowModified(true);
//     auto pixmapPath = m_secondarymodel->getSong(index.row())->pixmapPath();
//     static const QPixmap empty(":/defaults/no-image.webp");
//     QPixmap pixmap = empty;
//     if (!pixmapPath.isEmpty()) {
//         pixmap = QPixmap(pixmapPath);
//     }
//     pixmap = pixmap.scaled(ui->centralwidget->size()/4, Qt::KeepAspectRatio);
//     ui->songArt->setPixmap(pixmap);
//     QString title = m_secondarymodel->getSong(index.row())->title();
//     QString artist = m_secondarymodel->getSong(index.row())->artist();
//     ui->songLabel->setText(title);
//     ui->artistLabel->setText(artist);
//     ui->playSong->setEnabled(true);
//     // ui->stop->setEnabled(true);
// }

// void MainWindow::saveAppData() {
// if (!m_appSave.isFile()) setSaveFile(true, false);
// if (!m_appSave.isFile()) m_appSave = QFileInfo(m_gameDir.absoluteFilePath("save.msc"));
// QFileInfo savePath = m_appSave;
// qDebug() << "[MainWindow] Save start";
// qDebug() << QString("[MainWindow] Saving to: %1").arg(savePath.absoluteFilePath());
// QFile file(savePath.absoluteFilePath());
// if (!file.open(QIODevice::WriteOnly))
//     return;
// QDataStream out(&file);
// auto primaryItems = m_primarymodel->getItems();
// auto songs = m_musicStore->getSongs();
// // auto insertedList = m_gameManager->getAllInserted();
// m_undoStack->clear();
// out << primaryItems;
// out << songs;
// out << QVector<QString> {};
// out << m_musicStore->getMusicDir().absolutePath();
// setWindowModified(false);
// m_stickyModified = false;
// qDebug() << QString("[MainWindow] Saved %1 primary elements and %2 songs").arg(primaryItems.size()).arg(songs.size());
// }

// void MainWindow::loadAppData() {
//     qDebug() << "[MainWindow] Load start";
//     QFile file(m_appSave.absoluteFilePath());

//     if (!file.open(QIODevice::ReadOnly))
//         return;

//     QDataStream in(&file);
//     QVector<ListItem> primaryList;
//     QHash<QString, MusicObject> songsOwned;
//     QString musicPath;
//     QVector<QString> insertedList;
//     in >> primaryList >> songsOwned >> insertedList >> musicPath;
//     m_primarymodel->setItems(primaryList);
//     QHash<QString, std::shared_ptr<MusicObject>> songsShared;

//     for (auto it = songsOwned.constBegin(); it != songsOwned.constEnd(); it++) {
//         auto key = it.key();
//         auto value = it.value();
//         value.setStoragePath(musicPath);
//         qDebug() << QString("[MainWindow] Loading song key: %1, value: %2").arg(key, value.title());
//         songsShared.insert(key, std::make_shared<MusicObject>(value));
//     }

//     // m_gameManager->setInserted(insertedList);
//     m_musicStore->setSongs(songsShared);

//     if (m_musicStore->getMusicDir() != musicPath)
//         musicMismatch(!musicPath.isEmpty() && QDir(musicPath).exists(), musicPath);

//     int secondarySum = 0;
//     for (auto &prim : m_primarymodel->getItems()) {
//         qDebug() << QString("[MainWindow] Primary list has %1 elements").arg(prim.itemCount());
//         for (auto &sec : prim.getItems()){
//             qDebug() << QString("[MainWindow] List %1 title: %2 ").arg(prim.title(), sec.title());
//             QString hash = sec.getHash();
//             auto songPtr = m_musicStore->queryMusic(hash);
//             sec.setSong(songPtr);
//             secondarySum++;
//         }
//     }
//     qDebug() << QString("[MainWindow] Loaded %1 primary elements, %2 secondary elements and %3 songs").arg(m_primarymodel->getItems().length()).arg(secondarySum).arg(songsShared.size());
// }

void MainWindow::importDirectory(int type)
{
    QDir directory = FileManager::getInstance().getMusicPath();
    bool listType = true;
    static QStringList names{tr("CD1"), tr("CD2"), tr("CD3"), tr("Radio"), tr("Custom")};
    switch (type)
    {
    case LibraryController::DefinedDirs::CD1:
        // directory = m_gameDir;
        if (!directory.cd("CD1"))
        {
            qWarning() << "[MainWindow] Can't cd to requested directory";
            return;
        }
        break;
    case LibraryController::DefinedDirs::CD2:
        // directory = m_gameDir;
        if (!directory.cd("CD2"))
        {
            qWarning() << "[MainWindow] Can't cd to requested directory";
            return;
        }
        break;
    case LibraryController::DefinedDirs::CD3:
        // directory = m_gameDir;
        if (!directory.cd("CD3"))
        {
            qWarning() << "[MainWindow] Can't cd to requested directory";
            return;
        }
        break;
    case LibraryController::DefinedDirs::RADIO:
        // directory = m_gameDir;
        if (!directory.cd("Radio"))
        {
            qWarning() << "[MainWindow] Can't cd to requested directory";
            return;
        }
        listType = false;
        break;
    case LibraryController::DefinedDirs::OTHER:
        directory = getDir(LibraryController::CUSTOM);
        names[4] = directory.dirName().isEmpty() ? names.at(4) : directory.dirName();
        break;
    }
    m_progressBar->show();
    QUndoCommand *macro = new QUndoCommand(tr("Import directory"));
    new InsertCommand(m_primarymodel, names.at(type), listType, m_primarymodel->rowCount(), macro);
    new InsertCommand(m_secondarymodel, tr("Imported song"), listType,  0, macro);
    m_undoStack->push(macro);

    auto info = directory.entryInfoList();
    QStringList list;
    if (info.empty())
    {
        showWarningBox(tr("Nothing to import. Directory is empty"));
        return;
    }

    for (const auto &i : std::as_const(info)) {
        list << i.absoluteFilePath();
    }

    const auto songList = m_musicStore->importMusic(list);
    if (songList.isEmpty()) {
        qWarning() << "[MainWindow] Import failed or cancelled";
        return;
    }

    const auto indexes = ui->listItemView->selectionModel()->selectedIndexes();
    if (indexes.isEmpty()) {
        qWarning() << "[MainWindow] No selection to apply the import to";
        return;
    }

    const QModelIndex index = m_selectionState->currentSelection().indexes().constFirst();
    auto *listItem = m_secondarymodel->getItem();

    listItem->getItem(index.row())->setHash(songList.constFirst()->getHash());
    listItem->getItem(index.row())->setSong(songList.constFirst());

    for (int i = 1; i < songList.count(); ++i) {
        auto item = MusicItem(songList.at(i)->title(), songList.at(i));
        m_secondarymodel->insertRowInternal(index.row() + i, item);
    }
    ui->listItemView->setFocus();
    if (songList.count()) {
        setWindowModified(true);
        m_secondarymodel->revalidate(index.row());
        m_stickyModified = true;
    }
    m_progressBar->accept();
    ui->listItemView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
}


QListView *MainWindow::currentListView() const
{
    if (ui->listView->hasFocus())
        return ui->listView;

    if (ui->listItemView->hasFocus())
        return ui->listItemView;

    return nullptr;
}

void MainWindow::copy()
{
    auto view = currentListView();
    if (!view)
        return;

    auto model = view->model();
    auto indexes = view->selectionModel()->selectedRows();

    QMimeData *data = model->mimeData(indexes);
    QApplication::clipboard()->setMimeData(data);
}

void MainWindow::paste()
{
    auto view = currentListView();
    if (!view)
        return;
    const QMimeData *data = QApplication::clipboard()->mimeData();
    QString format = data->formats().constFirst();
    QByteArray binary = data->data(format);
    PasteCommand *cmd = new PasteCommand(view->model(), binary, format);
    m_undoStack->push(cmd);
}

void MainWindow::cut()
{
    auto view = currentListView();
    if (!view)
        return;
    auto model = view->model();
    auto selection = view->selectionModel()->selectedRows();
    QMimeData *mime = model->mimeData(selection);
    QString format = model->mimeTypes().constFirst();
    QByteArray data = mime->data(format);
    delete mime;
    CustomModelEdit *edit = view == ui->listView ? dynamic_cast<CustomModelEdit*>(m_primarymodel) : dynamic_cast<CustomModelEdit*>(m_secondarymodel);
    CutCommand *cmd = new CutCommand(edit, data, format, view->selectionModel()->selectedRows());
    m_undoStack->push(cmd);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!isWindowModified())
    {
        event->accept();
        return;
    }
    else
    {
        m_libraryController->saveAppData();
        event->accept();
    }
}

// void MainWindow::on_insertCD1_clicked()
// {
//     m_gameManager->insertSubdirToGame(m_secondarymodel->getSongs(), GameManager::CD1, m_secondarymodel->getItem()->getInsertHash());
//     ui->statusbar->showMessage(QString(tr("Inserted into %2").arg("CD1")), 3000);
//     setWindowModified(true);
//     m_stickyModified = true;
//     setInsertGroupBox();
// }

// void MainWindow::on_insertCD2_clicked()
// {
//     m_gameManager->insertSubdirToGame(m_secondarymodel->getSongs(), GameManager::CD2, m_secondarymodel->getItem()->getInsertHash());
//     ui->statusbar->showMessage(QString(tr("Inserted into %2").arg("CD2")), 3000);
//     setWindowModified(true);
//     m_stickyModified = true;
//     setInsertGroupBox();
// }

// void MainWindow::on_insertCD3_clicked()
// {
//     m_gameManager->insertSubdirToGame(m_secondarymodel->getSongs(), GameManager::CD3, m_secondarymodel->getItem()->getInsertHash());
//     ui->statusbar->showMessage(QString(tr("Inserted into %2").arg("CD3")), 3000);
//     setWindowModified(true);
//     m_stickyModified = true;
//     setInsertGroupBox();
// }

// void MainWindow::on_insertRadio_clicked()
// {
//     m_gameManager->insertSubdirToGame(m_secondarymodel->getSongs(), GameManager::RADIO, m_secondarymodel->getItem()->getInsertHash());
//     ui->statusbar->showMessage(QString(tr("Inserted into %2").arg("Radio")), 3000);
//     setWindowModified(true);
//     m_stickyModified = true;
//     setInsertGroupBox();
// }

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete)
    {
        if (ui->listView->hasFocus())
        {
            auto list = ui->listView->selectionModel()->selectedRows();
            auto *cmd = new RemoveCommand(m_primarymodel, list);
            m_undoStack->push(cmd);
        }
        else if (ui->listItemView->hasFocus())
        {
            auto list = m_selectionState->currentSelection().indexes();
            auto *cmd = new RemoveCommand(m_secondarymodel, list);
            m_undoStack->push(cmd);
        }
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::updateItemCountLabel(const ListItem *data)
{
    if (data == nullptr) {
        data = m_secondarymodel->getItem();
        if (data == nullptr)
            return;
    }
    int count = data->itemCount();
    int maximum = data->type() ? 15 : 199; // FIXME: Magic numbers
    ui->songAmountlabel->setText(QString("%1/%2").arg(count).arg(maximum));
    if (count > maximum)
        ui->songAmountlabel->setStyleSheet("color: red;");
    else
        ui->songAmountlabel->setStyleSheet("");
}

void MainWindow::on_importItem_clicked()
{
    const auto files = QFileDialog::getOpenFileNames(
        this,
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
            "All Files (*)"));
    m_progressBar->show();
    const auto songList = m_musicStore->importMusic(files);
    if (songList.isEmpty())
    {
        qWarning() << "[MainWindow] Import failed or cancelled";
        return;
    }

    const auto indexes = ui->listItemView->selectionModel()->selectedRows();
    if (indexes.isEmpty())
    {
        qWarning() << "[MainWindow] No selection to apply the import to";
        return;
    }

    const QModelIndex selection = indexes.constFirst();
    const QModelIndex index = m_secondarymodel->index(selection.row());
    auto *listItem = m_secondarymodel->getItem();

    listItem->getItem(index.row())->setHash(songList.constFirst()->getHash());
    listItem->getItem(index.row())->setSong(songList.constFirst());

    for (int i = 1; i < songList.count(); ++i)
    {
        auto item = MusicItem(songList.at(i)->title(), songList.at(i));
        m_secondarymodel->insertRowInternal(index.row() + i, item);
    }

    ui->listItemView->setFocus();
    if (songList.count()) {
        setWindowModified(true);
        m_secondarymodel->revalidate(index.row());
        m_stickyModified = true;
    }
    m_progressBar->accept();
    ui->listItemView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    // updateItemCountLabel();
    // songUpdated(index);
}

void MainWindow::musicMismatch(bool oldExists, const QDir &oldDir)
{
    // auto songsShared = m_musicStore->getSongsShared();
    // QMessageBox msgBox(this);
    // msgBox.setWindowTitle(tr("Music directory mismatch"));
    // msgBox.setText(tr("Saved music directory does not match the current one. Choose how to solve this issue."));
    // QPushButton *setToButton = nullptr;
    // QPushButton *copyButton = nullptr;
    // QPushButton *moveButton = nullptr;
    // QPushButton *deleteButton = nullptr;
    // if (oldExists)
    // {
    //     deleteButton = msgBox.addButton(tr("Delete old (New session)"), QMessageBox::ActionRole);
    //     if (oldDir.exists())
    //         setToButton = msgBox.addButton(tr("Set music direcory to the saved one"), QMessageBox::ActionRole);
    //     copyButton = msgBox.addButton(tr("Copy old to new"), QMessageBox::ActionRole);
    //     moveButton = msgBox.addButton(tr("Move old to new"), QMessageBox::ActionRole);
    // }
    // else
    // {
    //     deleteButton = msgBox.addButton(tr("New session"), QMessageBox::ActionRole);
    // }
    // QPushButton *abortButton = msgBox.addButton(QMessageBox::Abort);
    // msgBox.exec();
    // if (msgBox.clickedButton() == abortButton)
    // {
    //     exit(EXIT_FAILURE);
    // }
    // else if (copyButton != nullptr && msgBox.clickedButton() == copyButton)
    // {
    //     m_musicStore->copyAllSongs(m_musicStore->getMusicDir());
    // }
    // else if (moveButton != nullptr && msgBox.clickedButton() == moveButton)
    // {
    //     m_musicStore->moveAllSongs(m_musicStore->getMusicDir());
    // }
    // else if (deleteButton != nullptr && msgBox.clickedButton() == deleteButton)
    // {
    //     m_primarymodel->removeRows(0, m_primarymodel->rowCount());
    //     m_musicStore->uncheckedRemoveAll();
    //     m_musicStore->setSongs({});
    // }
    // else if (setToButton != nullptr && msgBox.clickedButton() == setToButton)
    // {
    //     m_musicStore->setMusicDir(oldDir);
    // }
    // else
    // {
    //     exit(EXIT_FAILURE);
    // }
    // m_musicStore->setSongs(songsShared);
}

// void MainWindow::importDirectory(int type)
// {
    // QDir directory;
    // bool listType = true;
    // QStringList names{tr("CD1"), tr("CD2"), tr("CD3"), tr("Radio"), tr("Custom")};

    // switch (type) {
    // case GameManager::CD1:
    //     directory = m_gameDir;
    //     if (!directory.cd("CD1")) {
    //         qWarning() << "[MainWindow] Can't cd to requested directory";
    //         return;
    //     }
    //     break;
    // case GameManager::CD2:
    //     directory = m_gameDir;
    //     if (!directory.cd("CD2")) {
    //         qWarning() << "[MainWindow] Can't cd to requested directory";
    //         return;
    //     }
    //     break;
    // case GameManager::CD3:
    //     directory = m_gameDir;
    //     if (!directory.cd("CD3")) {
    //         qWarning() << "[MainWindow] Can't cd to requested directory";
    //         return;
    //     }
    //     break;
    // case GameManager::RADIO:
    //     directory = m_gameDir;
    //     if (!directory.cd("Radio")) {
    //         qWarning() << "[MainWindow] Can't cd to requested directory";
    //         return;
    //     }
    //     listType = false;
    //     break;
    // case 4:
    //     directory = getDir(tr("Select directory for import"), QDir::homePath());;
    //     names[4] = directory.dirName().isEmpty() ? names.at(4) : directory.dirName();
    //     break;
    // }

    // QUndoCommand *macro = new QUndoCommand(tr("Import directory"));
    // new InsertPrimaryCommand(ui->listView, m_primarymodel, names.at(type), listType, m_primarymodel->rowCount(), false, macro);
    // new InsertSecondaryCommand(ui->listItemView, 0, macro);
    // m_undoStack->push(macro);

    // auto info = directory.entryInfoList();
    // QStringList list;
    // if (info.empty()) {
    //     showWarningBox(tr("Nothing to import. Directory is empty"));
    //     return;
    // }

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
    // const QModelIndex index = m_secondarymodel->index(selection.row());
    // auto *listItem = m_secondarymodel->getItem();

    // listItem->getItem(index.row())->setHash(songList.constFirst()->getHash());
    // listItem->getItem(index.row())->setSong(songList.constFirst());

    // for (int i = 1; i < songList.count(); ++i) {
    //     auto item = MusicItem(songList.at(i)->title(), songList.at(i));
    //     m_secondarymodel->insertRowInternal(index.row() + i, item);
    // }

    // setWindowModified(true);
    // m_stickyModified = true;
    // ui->listItemView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    // updateItemCountLabel();
    // songUpdated(index);
// }
