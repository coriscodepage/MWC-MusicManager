#ifndef MOVECOMMAND_H
#define MOVECOMMAND_H

#include "musicstorage.h"
#include <QUndoCommand>
#include <qabstractitemmodel.h>
#include <QMimeData>

class MoveCommand : public QUndoCommand
{
public:
    MoveCommand(QAbstractListModel *model, const QMimeData *data, int row, int count, const QModelIndexList &draggedIndexes);
    void undo() override;
    void redo() override;

private:
    QAbstractListModel *m_model;
    MusicStorage *m_musicStore;
    const QMimeData *m_data;
    int m_row;
    int m_count;
    QModelIndexList m_draggedIndexes;
};

#endif // MOVECOMMAND_H
