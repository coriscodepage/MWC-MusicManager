#ifndef INSERTSECONDARYCOMMAND_H
#define INSERTSECONDARYCOMMAND_H

#include <QUndoCommand>
#include <qlistview.h>

class InsertSecondaryCommand : public QUndoCommand
{
public:
    InsertSecondaryCommand(QListView *view, int row);
    void undo() override;
    void redo() override;

private:
    QListView *m_view;
    int m_row;
};

#endif // INSERTSECONDARYCOMMAND_H
