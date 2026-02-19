#ifndef MOVEPRIMARYCOMMAND_H
#define MOVEPRIMARYCOMMAND_H

#include "listitem.h"
#include "primarylistmodel.h"
#include <QUndoCommand>

class MovePrimaryCommand : public QUndoCommand
{
public:
    MovePrimaryCommand(PrimaryListModel *model, const QVector<ListItem> &movingItems, int sourceRow, int count, int destinationChild);
    void undo() override;
    void redo() override;

private:
    PrimaryListModel *m_model;
    const QVector<ListItem> m_movingItems;
    int m_sourceRow;
    int m_count;
    int m_destinationChild;
};

#endif // MOVEPRIMARYCOMMAND_H
