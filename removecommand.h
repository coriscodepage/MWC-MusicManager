#ifndef REMOVECOMMAND_H
#define REMOVECOMMAND_H

#include "custommodeledit.h"
#include <QUndoCommand>
#include <qlistview.h>

class RemoveCommand : public QUndoCommand
{
public:
    RemoveCommand(CustomModelEdit *itemModel, QModelIndexList indexes, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    CustomModelEdit *m_itemModel;
    QVariant m_data;
    QModelIndexList m_indexes;
    int m_row;
};

#endif // REMOVECOMMAND_H
