#ifndef INSERTCOMMAND_H
#define INSERTCOMMAND_H

#include <QUndoCommand>

class InsertCommand : public QUndoCommand
{
public:
    InsertCommand();
};

#endif // INSERTCOMMAND_H
