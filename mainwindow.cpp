#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "primarylistmodel.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_primarymodel = new PrimaryListModel(this);
    m_secondarymodel = new SecondaryListModel(this);
    ui->listView->setModel(m_primarymodel);
    ui->listItemView->setModel(m_secondarymodel);

    m_primarymodel->addItem(ListItem("meeow :3"));
    m_primarymodel->addItem(ListItem("mipiiii"));
    connect(ui->listView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MainWindow::handlePrimaryListSelectionChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::handlePrimaryListSelectionChanged(const QModelIndex &index) {
    if (!index.isValid()) return;
    qDebug() << index;
    ListItem &data = m_primarymodel->getItem(index);
    m_secondarymodel->setSource(&data);
}

void MainWindow::on_addCD_clicked()
{
    ui->listView->setFocus();
    m_primarymodel->addItem(ListItem("New list", true));
    QModelIndex index = m_primarymodel->index(m_primarymodel->rowCount()-1);
    ui->listView->edit(index);
    handlePrimaryListSelectionChanged(index);
}


void MainWindow::on_newItem_clicked()
{
    ui->listItemView->setFocus();
    m_secondarymodel->addItem(MusicItem("New song"));
}


void MainWindow::on_deleteList_clicked()
{
    auto selections = ui->listView->selectionModel()->selection().indexes();
    if (selections.empty()) return;
    auto selection = selections[0];
    if (selection.row() > 0) {
        handlePrimaryListSelectionChanged(selection);
        ui->listItemView->setFocus();
    } else if (m_primarymodel->rowCount() > 1) {
        QModelIndex index = m_primarymodel->index(m_primarymodel->rowCount()-1);
        handlePrimaryListSelectionChanged(index);
        ui->listItemView->setFocus();
    } else {
        m_secondarymodel->setSource(nullptr);
    }
    m_primarymodel->removeItem(selection.row());
}

