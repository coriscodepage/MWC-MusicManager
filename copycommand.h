#ifndef COPYCOMMAND_H
#define COPYCOMMAND_H

#include <QUndoCommand>

class CopyCommand : public QUndoCommand
{
public:
    CopyCommand();
};

#endif // COPYCOMMAND_H
