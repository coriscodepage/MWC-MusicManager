#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "musicstorage.h"
#include "primarylistmodel.h"
#include "secondarylistmodel.h"
#include <QMainWindow>
#include <QDir>
#include <QMediaPlayer>
#include <qlistview.h>

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
    void saveAppData();
    void loadAppData();

    void on_newItem_clicked();

    void on_deleteList_clicked();

    void on_downloadItem_clicked();

    void on_addRadio_clicked();

    void showListContextMenu(const QPoint &pos);

    void showListItemContextMenu(const QPoint &pos);

    void on_play_clicked();

    void on_stop_clicked();

private:
    Ui::MainWindow *ui;
    PrimaryListModel *m_primarymodel;
    SecondaryListModel *m_secondarymodel;
    QMediaPlayer *m_mediaPlayer;
    MusicStorage *m_musicStore;
    QDir m_gameDir;
    QDir m_appDir;
    void prepareWorkingDir();
    void prepareAppDir();
    void songSelected(const QModelIndex &index);
    void songUpdated(const QModelIndex &index);
    QListView *currentListView() const;
    void copy();
    void cut();
    void paste();

protected:
    void closeEvent(QCloseEvent *event) override;
};
#endif // MAINWINDOW_H
