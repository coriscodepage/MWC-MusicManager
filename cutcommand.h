#ifndef CUTCOMMAND_H
#define CUTCOMMAND_H

#include <QUndoCommand>
#include <qlistview.h>

class CutCommand : public QUndoCommand
{
public:
    CutCommand(QListView *view);
    void redo() override;
    void undo() override;

private:
    QListView *m_view;
    QModelIndexList m_indexes;
    QMimeData *m_mimeData;
};

#endif // CUTCOMMAND_H
