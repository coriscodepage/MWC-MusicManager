#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "gamemanager.h"
#include "musicstorage.h"
#include "primarylistmodel.h"
#include "secondarylistmodel.h"
#include <QMainWindow>
#include <QDir>
#include <QMediaPlayer>
#include <qlistview.h>
#include <qundostack.h>

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
    void updateItemCountLabel();
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

    void on_insertCD1_clicked();

    void on_insertCD2_clicked();

    void on_insertCD3_clicked();

    void on_insertRadio_clicked();


    void on_importItem_clicked();

private:
    Ui::MainWindow *ui;
    PrimaryListModel *m_primarymodel;
    SecondaryListModel *m_secondarymodel;
    QMediaPlayer *m_mediaPlayer;
    MusicStorage *m_musicStore;
    GameManager *m_gameManager;
    QDir m_gameDir;
    QFileInfo m_appSave;
    QUndoStack *m_undoStack;
    bool m_stickyModified;
    void prepareDirectories();
    void songSelected(const QModelIndex &index);
    void songUpdated(const QModelIndex &index);
    QListView *currentListView() const;
    void copy();
    void cut();
    void paste();
    void showInfoBox(const QString &message);
    void showWarningBox(const QString &message);
    void showErrorBox(const QString &message);
    QString getDir(const QString &message, const QString &defaultPath);
    bool setMusicDir(bool exp);
    bool setGameDir(bool exp);
    bool setSaveFile(bool exp, bool open = true);
    void setUiEnabled(bool enabled);
    void musicMismatch(bool oldExists, const QDir &oldDir);
    void songUnselected();
    void setInsertGroupBox();

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
};
#endif // MAINWINDOW_H
