#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "downloaddialog.h"
#include "primarylistmodel.h"
#include <QSettings>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    prepareWorkingDir();
    qDebug() << QString("Game path: %1").arg(m_gameDir.absolutePath());
    prepareAppDir();
    qDebug() << QString("Application path: %1").arg(m_appDir.absolutePath());
    qDebug() << QString("Music storage path: %1").arg(m_musicDir.absolutePath());
    m_primarymodel = new PrimaryListModel(this);
    m_secondarymodel = new SecondaryListModel(this);
    ui->listView->setModel(m_primarymodel);
    ui->listItemView->setModel(m_secondarymodel);

    connect(ui->listView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MainWindow::handlePrimaryListSelectionChanged);
    connect(ui->listItemView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MainWindow::handleSecondaryListSelectionChanged);
    connect(m_secondarymodel, &SecondaryListModel::dataChanged, this, &MainWindow::handleSecondaryListSelectionChanged);

    ui->listView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listView, &QListView::customContextMenuRequested, this, &MainWindow::showListContextMenu);
    ui->listItemView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listItemView, &QListView::customContextMenuRequested, this, &MainWindow::showListItemContextMenu);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::handlePrimaryListSelectionChanged(const QModelIndex &index, const QModelIndex &previous) {
    Q_UNUSED(previous);
    if (!index.isValid()) return;
    //qDebug() << QString("Primary: {%1}").arg(index.row());
    ListItem &data = m_primarymodel->getItem(index);
    m_secondarymodel->setSource(&data);
    ui->listItemView->setEnabled(true);
}

void MainWindow::handleSecondaryListSelectionChanged(const QModelIndex &index, const QModelIndex &previous) {
    Q_UNUSED(previous);
    if (!index.isValid()) return;
    //qDebug() << QString("Secondary: {%1}").arg(index.row());
    auto song = m_secondarymodel->getItem();
    ui->songLabel->setText(song->getItem(index.row()).title());
    songSelected(index);
}

void MainWindow::on_addCD_clicked() {
    ui->listView->setFocus();
    m_primarymodel->addItem(ListItem("New list", true));
    QModelIndex index = m_primarymodel->index(m_primarymodel->rowCount()-1);
    ui->listView->edit(index);
    ui->listView->setCurrentIndex(index);
    handlePrimaryListSelectionChanged(index, index);
}


void MainWindow::on_newItem_clicked() {
    m_secondarymodel->addItem(MusicItem("New song"));
    QModelIndex index = m_secondarymodel->index(m_secondarymodel->rowCount()-1);
    ui->listItemView->setCurrentIndex(index);
    ui->listItemView->setFocus();
}


void MainWindow::on_deleteList_clicked() {
    auto selections = ui->listView->selectionModel()->selection().indexes();
    if (selections.empty()) return;
    auto selection = selections[0];
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
    m_primarymodel->removeItem(selection);
}


void MainWindow::on_downloadItem_clicked() {
    auto dialog = DownloadDialog();
    int res = dialog.exec();
    if (res == QDialog::Accepted) {
        auto url = dialog.url();
        qDebug() << url;
        m_downloader.downloadSong(url, m_musicDir);
    }

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
            qWarning() << "Failed to create app subdirectory";
            return;
        }
    }
    if (!m_appDir.cd(subDirName)) {
        qWarning() << "Failed to enter app subdirectory";
        return;
    }

    if (!m_appDir.exists(musicDirName)) {
        if (!m_appDir.mkpath(musicDirName)) {
            qWarning() << "Failed to create music directory";
            return;
        }
    }
    m_musicDir = m_appDir;
    if (!m_musicDir.cd(musicDirName)) {
        qWarning() << "Failed to enter music subdirectory";
        return;
    }
}

void MainWindow::showListContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->listView->indexAt(pos);

    if (!index.isValid())
        return;

    QMenu contextMenu(tr("Context menu"), this);

    QAction action1("Edit", this);
    QAction action2("Delete", this);
    contextMenu.addAction(&action1);
    contextMenu.addAction(&action2);

    auto res = contextMenu.exec(ui->listView->viewport()->mapToGlobal(pos));
    if (res == &action1)
        ui->listView->edit(index);
    else if (res == &action2)
        m_primarymodel->removeItem(index);
}

void MainWindow::showListItemContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->listItemView->indexAt(pos);

    if (!index.isValid())
        return;

    QMenu contextMenu(tr("Context menu"), this);

    QAction action1("Edit", this);
    QAction action2("Delete", this);
    contextMenu.addAction(&action1);
    contextMenu.addAction(&action2);

    auto res = contextMenu.exec(ui->listItemView->viewport()->mapToGlobal(pos));
    if (res == &action1)
        ui->listItemView->edit(index);
    else if (res == &action2) {
        m_secondarymodel->removeItem(index);
        if (m_secondarymodel->rowCount() == 0) ui->downloadControls->setEnabled(false);
    }
}

void MainWindow::songSelected(const QModelIndex &index) {
    ui->downloadControls->setEnabled(true);

}
