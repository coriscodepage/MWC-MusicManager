#include "deleteprimarycommand.h"

DeletePrimaryCommand::DeletePrimaryCommand(MainWindow *window, const QModelIndexList &selections)
    : m_selections(selections)
    , m_window(window)
{

}

void DeletePrimaryCommand::undo() {

}

void DeletePrimaryCommand::redo() {

}
