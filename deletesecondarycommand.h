#ifndef DELETESECONDARYCOMMAND_H
#define DELETESECONDARYCOMMAND_H

#include <QUndoCommand>

class DeleteSecondaryCommand : public QUndoCommand
{
public:
    DeleteSecondaryCommand();
};

#endif // DELETESECONDARYCOMMAND_H
