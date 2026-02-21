#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "cutcommand.h"
#include "insertprimarycommand.h"
#include "insertsecondarycommand.h"
#include "pastecommand.h"
#include "primarylistmodel.h"
#include "removecommand.h"
#include <QSettings>
#include <QFileDialog>
#include <QAudioOutput>
#include <QCloseEvent>
#include <QClipboard>
#include <qmimedata.h>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_mediaPlayer = new QMediaPlayer(this);
    QAudioOutput* audioOutput = new QAudioOutput(this);
    audioOutput->setVolume(50);
    m_mediaPlayer->setAudioOutput(audioOutput);
    m_undoStack = new QUndoStack(this);
    m_musicStore = new MusicStorage(this);
    prepareDirectories();
    m_gameManager = new GameManager(m_gameDir);
    qDebug() << QString("[MainWindow] Game path: %1").arg(m_gameDir.absolutePath());
    qDebug() << QString("[MainWindow] Application path: %1").arg(m_appDir.absolutePath());
    m_primarymodel = new PrimaryListModel(this, m_musicStore, m_undoStack);
    m_secondarymodel = new SecondaryListModel(this, m_musicStore, m_undoStack);
    ui->listView->setModel(m_primarymodel);
    ui->listItemView->setModel(m_secondarymodel);

    connect(ui->actionSetGameDir, &QAction::triggered, this, [this]() {
        bool res = setGameDir(true);
        if(!m_gameDir.exists()) setUiEnabled(false);
        QSettings settings("Kori", "Music Manager");
        if ((!settings.contains("musicdir") && res) || res)
            musicMismatch(true);

    });
    connect(ui->actionSetAppDir, &QAction::triggered, this, [this]() {
        bool res = setAppDir(true);
        if(!m_appDir.exists()) setUiEnabled(false);
        QSettings settings("Kori", "Music Manager");
        if ((!settings.contains("musicdir") && res) || res)
            musicMismatch(true);
    });
    connect(ui->actionSetMusicDir, &QAction::triggered, this, [this]() {
        bool res = setMusicDir(true);
        if(!m_musicStore->getMusicDir().exists()) setUiEnabled(false);
        if((m_musicStore->getSongsShared().count() > 0 && res) || res)
            musicMismatch(true);
    });
    connect(ui->actionSetDirsDefault, &QAction::triggered, this, [this]() {
        QSettings settings("Kori", "Music Manager");
        settings.remove("musicdir");
        settings.remove("appdir");
        prepareDirectories();
    });

    connect(ui->listView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MainWindow::handlePrimaryListSelectionChanged);
    connect(ui->listItemView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MainWindow::handleSecondaryListSelectionChanged);
    connect(m_secondarymodel, &SecondaryListModel::dataChanged, this, &MainWindow::handleSecondaryListSelectionChanged);

    connect(m_secondarymodel, &QAbstractItemModel::rowsRemoved, this, [this]() {
        ui->downloadControls->setEnabled(m_secondarymodel->rowCount() > 0);
        updateItemCountLabel();
    });
    connect(m_primarymodel, &QAbstractItemModel::rowsRemoved, this, [this]() {
        m_secondarymodel->setSource(nullptr);
        updateItemCountLabel();

    });

    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listView, &QListView::customContextMenuRequested, this, &MainWindow::showListContextMenu);
    ui->listItemView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listItemView, &QListView::customContextMenuRequested, this, &MainWindow::showListItemContextMenu);

    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveAppData);
    connect(ui->actionCopy, &QAction::triggered, this, &MainWindow::copy);
    connect(ui->actionPaste, &QAction::triggered, this, &MainWindow::paste);
    connect(ui->actionCut, &QAction::triggered, this, &MainWindow::cut);

    connect(m_undoStack, &QUndoStack::cleanChanged, this, [this](bool clean) {
        if (!m_stickyModified)
            setWindowModified(!clean);
    });
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
    ui->listItemView->setDefaultDropAction(Qt::MoveAction);

    ui->listView->setDragEnabled(true);
    ui->listView->setAcceptDrops(true);
    ui->listView->setDropIndicatorShown(true);
    ui->listView->setDefaultDropAction(Qt::MoveAction);

    if(QFileInfo(m_appDir.filePath("data.msc")).isFile())
        loadAppData();

    ui->downloadControls->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::handlePrimaryListSelectionChanged(const QModelIndex &index, const QModelIndex &previous) {
    if (!index.isValid()) return;
    qDebug() << QString("[MainWindow] Primary: {%1}").arg(index.row());
    ListItem &data = m_primarymodel->getItem(index);
    m_secondarymodel->setSource(&data);
    ui->listItemView->setEnabled(true);
    updateItemCountLabel();
}

void MainWindow::handleSecondaryListSelectionChanged(const QModelIndex &index, const QModelIndex &previous) {
    Q_UNUSED(previous);
    if (!index.isValid()) return;
    qDebug() << QString("[MainWindow] Secondary: {%1}").arg(index.row());
    updateItemCountLabel();
    songSelected(index);
}

void MainWindow::on_addCD_clicked() {
    InsertPrimaryCommand *cmd = new InsertPrimaryCommand(ui->listView, m_primarymodel, "New list", true, m_primarymodel->rowCount());
    m_undoStack->push(cmd);
}


void MainWindow::on_newItem_clicked() {
    InsertSecondaryCommand *cmd = new InsertSecondaryCommand(ui->listItemView, m_secondarymodel->rowCount());
    m_undoStack->push(cmd);
}


void MainWindow::on_deleteList_clicked() {
    QUndoCommand *macro = new QUndoCommand("Remove List");
    auto indexes = ui->listView->selectionModel()->selectedIndexes();

    for (const QModelIndex &index : std::as_const(indexes)) {
        new RemoveCommand(ui->listView, index.row(), m_secondarymodel, macro);
    }
    m_undoStack->push(macro);
}


void MainWindow::on_downloadItem_clicked() {

    const auto songList = m_musicStore->downloadMusic(this);
    if (songList.isEmpty()) {
        qWarning() << "[MainWindow] Download failed or cancelled";
        return;
    }

    const auto indexes = ui->listItemView->selectionModel()->selectedIndexes();
    if (indexes.isEmpty()) {
        qWarning() << "[MainWindow] No selection to apply the download to";
        return;
    }

    const QModelIndex selection = indexes.constFirst();
    const QModelIndex index = m_secondarymodel->index(selection.row());
    auto *listItem = m_secondarymodel->getItem();

    listItem->getItem(index.row())->setHash(songList.constFirst()->getHash());
    listItem->getItem(index.row())->setSong(songList.constFirst());

    for (int i = 1; i < songList.count(); ++i) {
        auto item = MusicItem(songList.at(i)->title(), songList.at(i));
        m_secondarymodel->insertRowInternal(index.row() + i, item);
    }

    songUpdated(index);
    setWindowModified(true);
    m_stickyModified = true;
    ui->listItemView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
    updateItemCountLabel();
}


void MainWindow::on_addRadio_clicked() {
    InsertPrimaryCommand *cmd = new InsertPrimaryCommand(ui->listView, m_primarymodel, "New list", false, m_primarymodel->rowCount());
    m_undoStack->push(cmd);
}

void MainWindow::prepareDirectories() {
    QSettings settings("Kori", "Music Manager");
    for(auto &key : settings.allKeys()) {
        qDebug() << QString("[MainWindow] Setting with key %1 set to %2").arg(key, settings.value(key).toString());
    }
    setUiEnabled(false);
    setGameDir(false);
    setAppDir(false);
    setMusicDir(false);
    if (m_gameDir.exists())
        setUiEnabled(true);
}

QString MainWindow::getDir(const QString &message, const QString &defaultPath) {
    QString path = QFileDialog::getExistingDirectory(
        this,
        message,
        defaultPath,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    // if (path.isEmpty() || !QDir(path).exists()) {
    //     showWarningBox(tr("Select a valid directory"));
    // }
    return path;
}

void MainWindow::showInfoBox(const QString &message) {
    QMessageBox::information(this, tr("Information"), message);
}

void MainWindow::showWarningBox(const QString &message) {
    QMessageBox::warning(this, tr("Warning"), message);
}

void MainWindow::showErrorBox(const QString &message) {
    QMessageBox::critical(this, tr("Error"), message);
}

void MainWindow::setUiEnabled(bool enabled) {
    centralWidget()->setEnabled(enabled);
    menuBar()->setEnabled(enabled);
}

bool MainWindow::setMusicDir(bool exp) {
    QSettings settings("Kori", "Music Manager");
    QString musicDirName = "music";
    QDir musicDir = m_appDir;

    if (exp) {
        QString path = getDir(tr("Select music directory"), m_appDir.exists() ? m_appDir.absolutePath() : QDir::homePath());
        if (!path.isEmpty() && QDir(path).exists()) {
            settings.setValue("musicdir", QVariant::fromValue(path));
            musicDir = QDir(path);
        } else {
            showWarningBox(tr("Please select a valid directory."));
            return false;
        }
    } else {
        if (settings.contains("musicdir")) {
            auto path = settings.value("musicdir").toString();
            if (!path.isEmpty() && QDir(path).exists())
                musicDir = QDir(path);
            else {
                showWarningBox(tr("Music directory invalid. Defaulting to game directory. If that is incorrect change it in the settings."));
            }
        }
    }

    if (!musicDir.exists(musicDirName)) {
        if (!musicDir.mkpath(musicDirName)) {
            qWarning() << "[MainWindow] Failed to create music directory";
            return false;
        }
    }
    if (!musicDir.cd(musicDirName)) {
        qWarning() << "[MainWindow] Failed to enter music subdirectory";
        return false;
    }
    m_musicStore->setMusicDir(musicDir);
    return exp;
}

bool MainWindow::setGameDir(bool exp) {
    QSettings settings("Kori", "Music Manager");
    if (exp) {
        QDir dir;
        QString path;
        do {
            path = getDir(tr("Select game directory"), m_gameDir.exists() ? m_gameDir.absolutePath() : QDir::homePath());
            dir = QDir(path);
            if (dir.exists() && !path.isEmpty()) {
                if(dir.entryList().contains("mysummercar.exe") || dir.entryList().contains("mywintercar.exe")) { // INFO: Check if game is really here
                    QString saveVal = dir.absolutePath();
                    settings.setValue("gamedir", QVariant::fromValue(saveVal));
                    m_gameDir = dir;
                    setAppDir(false);
                    setMusicDir(false);
                    return true;
                }
            }
            showWarningBox(tr("Directory does not contain mysummercar.exe or mywintercar.exe, please select a valid directory."));
        } while(!dir.exists() || path.isEmpty());
    } else {
        if (settings.contains("gamedir")) {
            auto path = settings.value("gamedir").toString();
            if (!path.isEmpty() && QDir(path).exists()) {
                m_gameDir = QDir(path);
                return false;
            }
        }
        setGameDir(true);
    }
    return false;
}

bool MainWindow::setAppDir(bool exp) {
    QSettings settings("Kori", "Music Manager");
    QString subDirName = "musicManager";

    if(exp) {
        QString path = getDir(tr("Select application save directory"), m_appDir.exists() ? m_appDir.absolutePath() : QDir::homePath());
        if (!path.isEmpty() && QDir(path).exists()) {
            settings.setValue("appdir", QVariant::fromValue(path));
            m_appDir = QDir(path);
        } else {
            showWarningBox(tr("Please select a valid directory."));
            return false;
        }
    } else {
        if (settings.contains("appdir")) {
            auto path = settings.value("appdir").toString();
            if (!path.isEmpty() && QDir(path).exists()) {
                m_appDir = QDir(path);
                setMusicDir(false);
            } else {
                showWarningBox(tr("App directory invalid. Defaulting to game directory. If that is incorrect change it in the settings."));
                m_appDir = m_gameDir;
            }
        } else {
            m_appDir = m_gameDir;
        }
    }

    if (!m_appDir.exists(subDirName)) {
        if (!m_appDir.mkpath(subDirName)) {
            qWarning() << "[MainWindow] Failed to create app subdirectory";
            return false;
        }
    }
    if (!m_appDir.cd(subDirName)) {
        qWarning() << "[MainWindow] Failed to enter app subdirectory";
        return false;
    }
    return exp;
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
    contextMenu.addAction(&action1);
    contextMenu.addAction(&action2);
    contextMenu.addAction(&action3);
    contextMenu.addAction(&action4);
    contextMenu.addAction(&action5);

    auto res = contextMenu.exec(ui->listView->viewport()->mapToGlobal(pos));
    if (res == &action1)
        ui->listView->edit(index);
    else if (res == &action2) {
        QUndoCommand *macro = new QUndoCommand("Remove Lists");
        for (const QModelIndex &index : std::as_const(list))
            new RemoveCommand(ui->listView, index.row(), m_secondarymodel, macro);
        m_undoStack->push(macro);
    } else if (res == &action3) {
        QMimeData *data = m_primarymodel->mimeData(list);
        QApplication::clipboard()->setMimeData(data);
    } else if (res == &action4) {
        QMimeData *data = m_secondarymodel->mimeData(list);
        QByteArray binary = data->data(m_secondarymodel->mimeTypes().constFirst());
        CutCommand *cmd = new CutCommand(ui->listView, binary, m_secondarymodel->mimeTypes().constFirst());
        m_undoStack->push(cmd);
        int firstRow = list.first().row();
        int rowCount = list.count();
        m_secondarymodel->removeRows(firstRow, rowCount);
    } else if (res == &action5) {
        int firstRow = list.first().row();
        const QMimeData *data = QApplication::clipboard()->mimeData();
        QString format = data->formats().constFirst();
        QByteArray binary = data->data(format);
        PasteCommand *cmd = new PasteCommand(ui->listView, binary, format, firstRow);
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
    else if (res == &action2) {
        QUndoCommand *macro = new QUndoCommand("Remove Songs");
        for (const QModelIndex &index : std::as_const(list))
            new RemoveCommand(ui->listItemView, index.row(), nullptr, macro);
        m_undoStack->push(macro);
    } else if (res == &action3) {
        QMimeData *data = m_secondarymodel->mimeData(list);
        QApplication::clipboard()->setMimeData(data);
    } else if (res == &action4) {
        QMimeData *data = m_secondarymodel->mimeData(list);
        QByteArray binary = data->data(m_secondarymodel->mimeTypes().constFirst());
        CutCommand *cmd = new CutCommand(ui->listItemView, binary, m_secondarymodel->mimeTypes().constFirst());
        m_undoStack->push(cmd);
        int firstRow = list.first().row();
        int rowCount = list.count();
        m_secondarymodel->removeRows(firstRow, rowCount);
    } else if (res == &action5) {
        int firstRow = list.first().row();
        const QMimeData *data = QApplication::clipboard()->mimeData();
        QString format = data->formats().constFirst();
        QByteArray binary = data->data(format);
        PasteCommand *cmd = new PasteCommand(ui->listItemView, binary, format, firstRow);
        m_undoStack->push(cmd);
    }
}

void MainWindow::songSelected(const QModelIndex &index) {
    ui->downloadControls->setEnabled(true);
    auto pixmap = m_secondarymodel->getPixmap(index.row());
    pixmap = pixmap.scaled(ui->centralwidget->size()/4, Qt::KeepAspectRatio);
    ui->songArt->setPixmap(pixmap);
    ui->songLabel->setText(m_secondarymodel->getTitle(index.row()));
}

void MainWindow::songUpdated(const QModelIndex &index) {
    setWindowModified(true);
    auto pixmap = m_secondarymodel->getPixmap(index.row());
    pixmap = pixmap.scaled(ui->centralwidget->size()/4, Qt::KeepAspectRatio);
    ui->songArt->setPixmap(pixmap);
    QString title = m_secondarymodel->getTitle(index.row());
    ui->songLabel->setText(title);
}

void MainWindow::on_play_clicked() {
    auto selections = ui->listItemView->selectionModel()->selection().indexes();
    if (selections.empty()) return;
    auto selection = selections.first();
    auto musicPath = m_secondarymodel->getSongPath(selection.row());
    m_mediaPlayer->setSource(musicPath);
    m_mediaPlayer->play();
}


void MainWindow::on_stop_clicked() {
    m_mediaPlayer->stop();
}

void MainWindow::saveAppData() {
    QFileInfo savePath = QFileInfo(m_appDir.filePath("data.msc"));
    qDebug() << "[MainWindow] Save start";
    qDebug() << QString("[MainWindow] Saving to: %1").arg(savePath.absoluteFilePath());
    QFile file(savePath.absoluteFilePath());
    if (!file.open(QIODevice::WriteOnly))
        return;
    QDataStream out(&file);
    auto primaryItems = m_primarymodel->getItems();
    auto songs = m_musicStore->getSongs();
    m_undoStack->clear();
    out << primaryItems;
    out << songs;
    out << m_musicStore->getMusicDir().absolutePath();
    setWindowModified(false);
    m_stickyModified = false;
    qDebug() << QString("[MainWindow] Saved %1 primary elements and %2 songs").arg(primaryItems.size()).arg(songs.size());
}

void MainWindow::loadAppData() {
    qDebug() << "[MainWindow] Load start";
    QFile file(m_appDir.filePath("data.msc"));

    if (!file.open(QIODevice::ReadOnly))
        return;

    QDataStream in(&file);
    QVector<ListItem> primaryList;
    QHash<QString, MusicObject> songsOwned;
    QString musicPath;
    in >> primaryList >> songsOwned >> musicPath;
    m_primarymodel->setItems(primaryList);
    QHash<QString, std::shared_ptr<MusicObject>> songsShared;

    for (auto it = songsOwned.constBegin(); it != songsOwned.constEnd(); it++) {
        auto key = it.key();
        auto value = it.value();
        value.setStoragePath(musicPath);
        qDebug() << QString("[MainWindow] Loading song key: %1, value: %2").arg(key, value.title());
        songsShared.insert(key, std::make_shared<MusicObject>(value));
    }

    m_musicStore->setSongs(songsShared);

    if (m_musicStore->getMusicDir() != musicPath)
        musicMismatch(!musicPath.isEmpty() && QDir(musicPath).exists());

    int secondarySum = 0;
    for (auto &prim : m_primarymodel->getItems()) {
        qDebug() << QString("[MainWindow] Primary list has %1 elements").arg(prim.itemCount());
        for (auto &sec : prim.getItems()){
            qDebug() << QString("[MainWindow] List %1 title: %2 ").arg(prim.title(), sec.title());
            QString hash = sec.getHash();
            auto songPtr = m_musicStore->queryMusic(hash);
            sec.setSong(songPtr);
            secondarySum++;
        }
    }
    qDebug() << QString("[MainWindow] Loaded %1 primary elements, %2 secondary elements and %3 songs").arg(m_primarymodel->getItems().length()).arg(secondarySum).arg(songsShared.size());
}

QListView *MainWindow::currentListView() const {
    if (ui->listView->hasFocus())
        return ui->listView;

    if (ui->listItemView->hasFocus())
        return ui->listItemView;

    return nullptr;
}

void MainWindow::copy() {
    auto view = currentListView();
    if (!view)
        return;

    auto model = view->model();
    auto indexes = view->selectionModel()->selectedIndexes();

    QMimeData *data = model->mimeData(indexes);
    QApplication::clipboard()->setMimeData(data);
}

void MainWindow::paste() {
    auto view = currentListView();
    if (!view)
        return;
    const QMimeData *data = QApplication::clipboard()->mimeData();
    QString format = data->formats().constFirst();
    QByteArray binary = data->data(format);
    PasteCommand *cmd = new PasteCommand(view, binary, format);
    m_undoStack->push(cmd);

}

void MainWindow::cut() {
    auto view = currentListView();
    if (!view)
        return;
    auto model = view->model();
    auto selection = view->selectionModel()->selectedIndexes();
    QMimeData *mime = model->mimeData(selection);
    QString format = model->mimeTypes().constFirst();
    QByteArray data = mime->data(format);
    delete mime;
    CutCommand *cmd = new CutCommand(view, data, format);
    m_undoStack->push(cmd);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!isWindowModified()) {
        event->accept();
        return;
    } else {
        saveAppData();
        event->accept();
    }


}

void MainWindow::on_insertCD1_clicked()
{
    m_gameManager->insertSubdirToGame(m_secondarymodel->getSongs(), "CD1");
    ui->statusbar->showMessage(QString(tr("Inserted into %2").arg("CD1")), 3000);
}


void MainWindow::on_insertCD2_clicked()
{
    m_gameManager->insertSubdirToGame(m_secondarymodel->getSongs(), "CD2");
    ui->statusbar->showMessage(QString(tr("Inserted into %2").arg("CD2")), 3000);
}

void MainWindow::on_insertCD3_clicked()
{
    m_gameManager->insertSubdirToGame(m_secondarymodel->getSongs(), "CD3");
    ui->statusbar->showMessage(QString(tr("Inserted into %2").arg("CD3")), 3000);
}

void MainWindow::on_insertRadio_clicked()
{
    m_gameManager->insertSubdirToGame(m_secondarymodel->getSongs(), "Radio");
    ui->statusbar->showMessage(QString(tr("Inserted into %2").arg("Radio")), 3000);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete) {
        if (ui->listView->hasFocus()) {
            auto list = ui->listView->selectionModel()->selectedIndexes();
            QUndoCommand *macro = new QUndoCommand("Remove Lists");
            for (const QModelIndex &index : std::as_const(list))
                new RemoveCommand(ui->listView, index.row(), m_secondarymodel, macro);
            m_undoStack->push(macro);
        } else if (ui->listItemView->hasFocus()) {
            auto list = ui->listItemView->selectionModel()->selectedIndexes();
            QUndoCommand *macro = new QUndoCommand("Remove Songs");
            for (const QModelIndex &index : std::as_const(list))
                new RemoveCommand(ui->listItemView, index.row(), nullptr, macro);
            m_undoStack->push(macro);
        }

    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::updateItemCountLabel() {
    auto data = m_secondarymodel->getItem();
    if (data == nullptr) return;
    int count = data->itemCount();
    int maximum = data->type() ? 15 : 199; //FIXME: Magic numbers
    ui->songAmountlabel->setText(QString("%1/%2").arg(count).arg(maximum));
    if (count > maximum)
        ui->songAmountlabel->setStyleSheet("color: red;");
    else
        ui->songAmountlabel->setStyleSheet("");
}


void MainWindow::on_importItem_clicked()
{
    m_musicStore->importMusic(this);
}

void MainWindow::musicMismatch(bool oldExists) {
    auto songsShared = m_musicStore->getSongsShared();
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Music directory mismatch"));
    msgBox.setText(tr("Saved music directory does not match the current one. Choose how to solve this issue."));
    QPushButton *copyButton =   nullptr;
    QPushButton *moveButton =   nullptr;
    QPushButton *deleteButton = nullptr;
    if (oldExists) {
        copyButton = msgBox.addButton(tr("Copy old to new"), QMessageBox::ActionRole);
        moveButton = msgBox.addButton(tr("Move old to new"), QMessageBox::ActionRole);
        deleteButton = msgBox.addButton(tr("Delete old (New session)"), QMessageBox::ActionRole);
    } else {
        deleteButton = msgBox.addButton(tr("New session"), QMessageBox::ActionRole);
    }
    QPushButton *abortButton = msgBox.addButton(QMessageBox::Abort);
    msgBox.exec();
    if(msgBox.clickedButton() == abortButton) {
        exit(EXIT_FAILURE);
    } else if (copyButton != nullptr && msgBox.clickedButton() == copyButton) {
        m_musicStore->copyAllSongs(m_musicStore->getMusicDir());
    } else if (moveButton != nullptr && msgBox.clickedButton() == moveButton) {
        m_musicStore->moveAllSongs(m_musicStore->getMusicDir());
    } else if (deleteButton || moveButton != nullptr && msgBox.clickedButton() == deleteButton) {
        m_primarymodel->removeRows(0, m_primarymodel->rowCount());
        m_musicStore->uncheckedRemoveAll();
        m_musicStore->setSongs({});
    } else {
        exit(EXIT_FAILURE);
    }
    m_musicStore->setSongs(songsShared);
}
