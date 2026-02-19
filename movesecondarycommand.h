#ifndef MOVESECONDARYCOMMAND_H
#define MOVESECONDARYCOMMAND_H

#include "secondarylistmodel.h"
#include <QUndoCommand>

class MoveSecondaryCommand : public QUndoCommand
{
public:
    MoveSecondaryCommand(SecondaryListModel *model, const QVector<MusicItem> &movingItems, int sourceRow, int count, int destinationChild);
    void undo() override;
    void redo() override;

private:
    SecondaryListModel *m_model;
    const QVector<MusicItem> m_movingItems;
    int m_sourceRow;
    int m_count;
    int m_destinationChild;
};

#endif // MOVESECONDARYCOMMAND_H
