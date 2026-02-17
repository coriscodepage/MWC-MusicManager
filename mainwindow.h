#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "download.h"
#include "primarylistmodel.h"
#include "secondarylistmodel.h"
#include <QMainWindow>
#include <QDir>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_addCD_clicked();
    void handlePrimaryListSelectionChanged(const QModelIndex &index, const QModelIndex &previous);
    void handleSecondaryListSelectionChanged(const QModelIndex &index, const QModelIndex &previous);

    void on_newItem_clicked();

    void on_deleteList_clicked();

    void on_downloadItem_clicked();

    void on_addRadio_clicked();

    void showListContextMenu(const QPoint &pos);

    void showListItemContextMenu(const QPoint &pos);

private:
    Ui::MainWindow *ui;
    PrimaryListModel *m_primarymodel;
    SecondaryListModel *m_secondarymodel;
    Downloader m_downloader;
    QDir m_gameDir;
    QDir m_appDir;
    QDir m_musicDir;
    void prepareWorkingDir();
    void prepareAppDir();
    void songSelected(const QModelIndex &index);
};
#endif // MAINWINDOW_H
