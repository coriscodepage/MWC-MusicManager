#ifndef TOGGLETYPECOMMAND_H
#define TOGGLETYPECOMMAND_H

#include "primarylistmodel.h"
#include <QUndoCommand>

class ToggleTypeCommand : public QUndoCommand {
public:
    ToggleTypeCommand(PrimaryListModel *model, const QModelIndex &index);
    void redo() override;
    void undo() override;

private:
    PrimaryListModel *m_model;
    QPersistentModelIndex m_index;
};

#endif // TOGGLETYPECOMMAND_H
