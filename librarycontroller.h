#ifndef LIBRARYCONTROLLER_H
#define LIBRARYCONTROLLER_H

#include "insertcontroller.h"
#include "primarylistmodel.h"
#include "secondarylistmodel.h"
#include <QObject>

class LibraryController : public QObject
{
    Q_OBJECT
public:
    enum DirType {
        GAME = 0,
        APP = 1,
        MUSIC = 2,
        CUSTOM = 3,
    };

    enum DefinedDirs {
        CD1 = 0,
        CD2 = 1,
        CD3 = 2,
        RADIO = 3,
        OTHER = 4,
    };
    explicit LibraryController(PrimaryListModel *listModel, SecondaryListModel *songModel, SelectionState *selectionState, MusicStorage *musicStore, InsertController *insertController, QUndoStack *undoStack, QObject *parent = nullptr);
    void loadAppData();
    void handleDirecorySelected(DirType type, const QString path);
    void saveAppData();

private:
    PrimaryListModel *m_listModel;
    SecondaryListModel *m_songModel;
    SelectionState *m_selectionState;
    MusicStorage *m_musicStore;
    InsertController *m_insertController;
    QUndoStack *m_undoStack;
    void prepareDirectories();
signals:
    void getDirectory(LibraryController::DirType type);
    void directoryInvalid(LibraryController::DirType type, bool retrym);
};

#endif // LIBRARYCONTROLLER_H
