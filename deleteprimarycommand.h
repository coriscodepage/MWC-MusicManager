#ifndef DELETEPRIMARYCOMMAND_H
#define DELETEPRIMARYCOMMAND_H

#include "mainwindow.h"
#include <QUndoCommand>
#include <qabstractitemmodel.h>

class DeletePrimaryCommand : public QUndoCommand
{
public:
    DeletePrimaryCommand(MainWindow *window, const QModelIndexList &selections);
    void undo() override;
    void redo() override;

private:
    QModelIndexList m_selections;
    MainWindow *m_window;
    ListItem m_data;
};

#endif // DELETEPRIMARYCOMMAND_H
