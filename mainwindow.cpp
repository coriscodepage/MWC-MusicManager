#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "cutcommand.h"
#include "pastecommand.h"
#include "primarylistmodel.h"
#include <QSettings>
#include <QFileDialog>
#include <QAudioOutput>
#include <QCloseEvent>
#include <QClipboard>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_mediaPlayer = new QMediaPlayer(this);
    m_musicStore = new MusicStorage(this);
    QAudioOutput* audioOutput = new QAudioOutput(this);
    audioOutput->setVolume(50);
    m_mediaPlayer->setAudioOutput(audioOutput);
    m_undoStack = new QUndoStack(this);

    prepareWorkingDir();
    m_gameManager = new GameManager(m_gameDir);
    qDebug() << QString("[MainWindow] Game path: %1").arg(m_gameDir.absolutePath());
    prepareAppDir();
    qDebug() << QString("[MainWindow] Application path: %1").arg(m_appDir.absolutePath());
    m_primarymodel = new PrimaryListModel(this, m_musicStore, m_undoStack);
    m_secondarymodel = new SecondaryListModel(this, m_musicStore, m_undoStack);
    ui->listView->setModel(m_primarymodel);
    ui->listItemView->setModel(m_secondarymodel);

    connect(ui->listView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MainWindow::handlePrimaryListSelectionChanged);
    connect(ui->listItemView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MainWindow::handleSecondaryListSelectionChanged);
    connect(m_secondarymodel, &SecondaryListModel::dataChanged, this, &MainWindow::handleSecondaryListSelectionChanged);

    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listView, &QListView::customContextMenuRequested, this, &MainWindow::showListContextMenu);
    ui->listItemView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listItemView, &QListView::customContextMenuRequested, this, &MainWindow::showListItemContextMenu);

    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveAppData);
    connect(ui->actionCopy, &QAction::triggered, this, &MainWindow::copy);
    connect(ui->actionPaste, &QAction::triggered, this, &MainWindow::paste);
    connect(ui->actionCut, &QAction::triggered, this, &MainWindow::cut);

    // connect(m_undoStack, &QUndoStack::cleanChanged, this, [this](bool clean) {
    //     setWindowModified(!clean);
    // });
    QAction *action_undo = m_undoStack->createUndoAction(this, "Undo");
    QAction *action_redo = m_undoStack->createRedoAction(this, "Redo");
    action_undo->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Z));
    action_undo->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::EditUndo));
    action_redo->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Y));
    action_redo->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::EditRedo));
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
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::handlePrimaryListSelectionChanged(const QModelIndex &index, const QModelIndex &previous) {
    Q_UNUSED(previous);
    if (!index.isValid()) return;
    qDebug() << QString("Primary: {%1}").arg(index.row());
    ListItem &data = m_primarymodel->getItem(index);
    m_secondarymodel->setSource(&data);
    ui->listItemView->setEnabled(true);
}

void MainWindow::handleSecondaryListSelectionChanged(const QModelIndex &index, const QModelIndex &previous) {
    Q_UNUSED(previous);
    if (!index.isValid()) return;
    qDebug() << QString("Secondary: {%1}").arg(index.row());
    songSelected(index);
}

void MainWindow::on_addCD_clicked() {
    setWindowModified(true);
    ui->listView->setFocus();
    m_primarymodel->addItem(ListItem("New list", true));
    QModelIndex index = m_primarymodel->index(m_primarymodel->rowCount()-1);
    ui->listView->edit(index);
    ui->listView->setCurrentIndex(index);
    handlePrimaryListSelectionChanged(index, index);
}


void MainWindow::on_newItem_clicked() {
    setWindowModified(true);
    m_secondarymodel->insertRow(m_secondarymodel->rowCount());
    QModelIndex index = m_secondarymodel->index(m_secondarymodel->rowCount()-1);
    ui->listItemView->setCurrentIndex(index);
    ui->listItemView->setFocus();
}


void MainWindow::on_deleteList_clicked() {
    //setWindowModified(true);
    auto selections = ui->listView->selectionModel()->selection().indexes();
    if (selections.empty()) return;
    QModelIndex selection = selections.constFirst();
    if (selection.row() > 0) {
        handlePrimaryListSelectionChanged(selection, selection);
        ui->listItemView->setFocus();
    } else if (m_primarymodel->rowCount() > 1) {
        QModelIndex index = m_primarymodel->index(m_primarymodel->rowCount()-1);
        handlePrimaryListSelectionChanged(index, index);
        ui->listItemView->setFocus();
    } else {
        m_secondarymodel->setSource(nullptr);
        ui->listItemView->setEnabled(false);
        ui->downloadControls->setEnabled(false);
    }

    for (auto &i : m_primarymodel->getItem(selection).getItems()) {
        QString hash = i.getHash();
        i.setSong(nullptr);
        m_musicStore->checkedRemoveSong(hash);
    }

    m_primarymodel->removeItem(selection);
}


void MainWindow::on_downloadItem_clicked() {
    setWindowModified(true);
    auto song = m_musicStore->downloadMusic(this);
    if (song == nullptr) {
        qWarning() << "[MainWindow] Download failed";
        return;
    }
    auto selections = ui->listItemView->selectionModel()->selection().indexes();
    if (selections.empty()) return; // TODO: In cases like this some logging would be in order.
    QModelIndex selection = selections.constFirst();
    QModelIndex index = m_secondarymodel->index(selection.row());
    auto listItem = m_secondarymodel->getItem();
    listItem->getItem(index.row())->setHash(song->getHash());
    listItem->getItem(index.row())->setSong(song);
    songUpdated(index);
}


void MainWindow::on_addRadio_clicked() {
    ui->listView->setFocus();
    m_primarymodel->addItem(ListItem("New list", false));
    QModelIndex index = m_primarymodel->index(m_primarymodel->rowCount()-1);
    ui->listView->edit(index);
    handlePrimaryListSelectionChanged(index, index);
}

void MainWindow::prepareWorkingDir() {
    QSettings settings("Kori", "Music Manager");
    //settings.remove("gamedir");
    if (settings.contains("gamedir")) {
        auto dir = settings.value("gamedir").toString();
        m_gameDir = dir;
        return;
    }

    QDir dir = QFileDialog::getExistingDirectory(
        this,
        "Select Music Directory",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    if (dir.exists()) {
        QString saveVal = dir.absolutePath();
        settings.setValue("gamedir", QVariant().fromValue(saveVal));
        m_gameDir = dir;
    }
}

void MainWindow::prepareAppDir() {
    QSettings settings("Kori", "Music Manager");
    if (settings.contains("appdir")) {
        auto dir = settings.value("gamedir").toString();
        m_appDir = dir;
    } else {
        m_appDir = m_gameDir;
    }

    QString subDirName = "musicManager";
    QString musicDirName = "music";

    if (!m_appDir.exists(subDirName)) {
        if (!m_appDir.mkpath(subDirName)) {
            qWarning() << "[MainWindow] Failed to create app subdirectory";
            return;
        }
    }
    if (!m_appDir.cd(subDirName)) {
        qWarning() << "[MainWindow] Failed to enter app subdirectory";
        return;
    }

    if (!m_appDir.exists(musicDirName)) {
        if (!m_appDir.mkpath(musicDirName)) {
            qWarning() << "[MainWindow] Failed to create music directory";
            return;
        }
    }
    QDir musicDir = m_appDir;
    if (!musicDir.cd(musicDirName)) {
        qWarning() << "[MainWindow] Failed to enter music subdirectory";
        return;
    }
    m_musicStore->setMusicDir(musicDir);
}

void MainWindow::showListContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->listView->indexAt(pos);
    QModelIndexList list;
    list << index;

    if (!index.isValid())
        return;

    QMenu contextMenu(tr("Context menu"), this);

    QAction action1("Edit", this);
    QAction action2("Delete", this);
    QAction action3("Copy", this);
    QAction action4("Cut", this);
    QAction action5("Paste", this);
    contextMenu.addAction(&action1);
    contextMenu.addAction(&action2);
    contextMenu.addAction(&action3);
    contextMenu.addAction(&action4);
    contextMenu.addAction(&action5);

    auto res = contextMenu.exec(ui->listView->viewport()->mapToGlobal(pos));
    if (res == &action1)
        ui->listView->edit(index);
    else if (res == &action2) {
        m_secondarymodel->setSource(nullptr);
        m_primarymodel->removeItem(index);
    } else if (res == &action3) {
        QMimeData *data = m_primarymodel->mimeData(list);
        QApplication::clipboard()->setMimeData(data);
    } else if (res == &action4) {
        QMimeData *data = m_primarymodel->mimeData(list);
        QApplication::clipboard()->setMimeData(data);
        int firstRow = list.first().row();
        int rowCount = list.count();
        m_primarymodel->removeRows(firstRow, rowCount);
        m_secondarymodel->setSource(nullptr);
    } else if (res == &action5) {
        m_primarymodel->removeRow(index.row());
        const QMimeData *data = QApplication::clipboard()->mimeData();
        m_primarymodel->dropMimeData(data, Qt::CopyAction, index.row(), 0, QModelIndex());
        ui->listView->setCurrentIndex(index);
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

    QAction action1("Edit", this);
    QAction action2("Delete", this);
    QAction action3("Copy", this);
    QAction action4("Cut", this);
    QAction action5("Paste", this);
    contextMenu.addAction(&action1);
    contextMenu.addAction(&action2);
    contextMenu.addAction(&action3);
    contextMenu.addAction(&action4);
    contextMenu.addAction(&action5);

    auto res = contextMenu.exec(ui->listItemView->viewport()->mapToGlobal(pos));
    if (res == &action1)
        ui->listItemView->edit(index);
    else if (res == &action2) {
        m_secondarymodel->removeRow(index.row());
        if (m_secondarymodel->rowCount() == 0) ui->downloadControls->setEnabled(false);
    } else if (res == &action3) {
        QMimeData *data = m_secondarymodel->mimeData(list);
        QApplication::clipboard()->setMimeData(data);
    } else if (res == &action4) {
        QMimeData *data = m_secondarymodel->mimeData(list);
        QApplication::clipboard()->setMimeData(data);

        int firstRow = list.first().row();
        int rowCount = list.count();
        m_secondarymodel->removeRows(firstRow, rowCount);
    } else if (res == &action5) {
        m_secondarymodel->removeRow(index.row());
        const QMimeData *data = QApplication::clipboard()->mimeData();
        m_secondarymodel->dropMimeData(data, Qt::CopyAction, index.row(), 0, QModelIndex());
        ui->listItemView->setCurrentIndex(index);
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
    m_musicStore->importMusic(QDir());
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
    setWindowModified(false);
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
    in >> primaryList >> songsOwned;
    m_primarymodel->setItems(primaryList);
    QHash<QString, std::shared_ptr<MusicObject>> songsShared;

    for (auto it = songsOwned.constBegin(); it != songsOwned.constEnd(); it++) {
        auto key = it.key();
        auto value = it.value();
        value.setStoragePath(m_musicStore->getMusicDir());
        qDebug() << QString("[MainWindow] Loading song key: %1, value: %2").arg(key, value.title());
        songsShared.insert(key, std::make_shared<MusicObject>(value));
    }

    m_musicStore->setSongs(songsShared);
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
    PasteCommand *cmd = new PasteCommand(view, data);
    m_undoStack->push(cmd);

}

void MainWindow::cut() {
    auto view = currentListView();
    if (!view)
        return;

    CutCommand *cmd = new CutCommand(view);
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
}


void MainWindow::on_insertCD2_clicked()
{
    m_gameManager->insertSubdirToGame(m_secondarymodel->getSongs(), "CD2");
}

void MainWindow::on_insertCD3_clicked()
{
    m_gameManager->insertSubdirToGame(m_secondarymodel->getSongs(), "CD3");
}

void MainWindow::on_insertRadio_clicked()
{
    m_gameManager->insertSubdirToGame(m_secondarymodel->getSongs(), "Radio");
}



