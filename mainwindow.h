#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "insertcontroller.h"
#include "librarycontroller.h"
#include "mediaplayer.h"
#include "musicstorage.h"
#include "primarylistmodel.h"
#include "progressdialog.h"
#include "secondarylistmodel.h"
#include "selectioncontroller.h"
#include "selectionstate.h"
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
    void updateItemCountLabel(const ListItem *data);
    // void saveAppData();
    // void loadAppData();

    void on_newItem_clicked();

    void on_deleteList_clicked();

    void on_downloadItem_clicked();

    void on_addRadio_clicked();

    void showListContextMenu(const QPoint &pos);

    void showListItemContextMenu(const QPoint &pos);

    void on_importItem_clicked();

private:
    Ui::MainWindow *ui;
    PrimaryListModel *m_primarymodel;
    SecondaryListModel *m_secondarymodel;
    MediaPlayer *m_mediaPlayer;
    MusicStorage *m_musicStore;
    SelectionController *m_selectionController;
    InsertController *m_insertController;
    SelectionState *m_selectionState;
    LibraryController *m_libraryController;
    QUndoStack *m_undoStack;
    ProgressDialog *m_progressBar;

    bool m_stickyModified;
    void prepareDirectories(); // FIXME: This cannot be in main.
    // void songSelected(const QModelIndex &index);
    // void songUpdated(const QModelIndex &index);
    QListView *currentListView() const;
    void copy();
    void cut();
    void paste();
    void showInfoBox(const QString &message);
    void showWarningBox(const QString &message);
    void showErrorBox(const QString &message);
    QString getDir(LibraryController::DirType type);
    // bool setMusicDir(bool exp);
    // bool setGameDir(bool exp);
    bool setSaveFile(bool exp, bool open = true);
    void setUiEnabled(bool enabled);
    void musicMismatch(bool oldExists, const QDir &oldDir);
    // void setInsertGroupBox();
    void importDirectory(int type);

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
};
#endif // MAINWINDOW_H
