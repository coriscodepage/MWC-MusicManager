#ifndef MOVECOMMAND_H
#define MOVECOMMAND_H

#include "custommodeledit.h"
#include <QUndoCommand>

class MoveCommand : public QUndoCommand
{
public:
    MoveCommand(CustomModelEdit *model, const QVector<QVariant> &movingItems, int sourceRow, int count, int destinationChild);
    void undo() override;
    void redo() override;

private:
    CustomModelEdit *m_model;
    const QVector<QVariant> m_movingItems;
    int m_sourceRow;
    int m_count;
    int m_destinationChild;
};

#endif // MOVECOMMAND_H
