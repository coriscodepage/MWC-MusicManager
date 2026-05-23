#ifndef INSERTCOMMAND_H
#define INSERTCOMMAND_H

#include "custommodeledit.h"
#include <QUndoCommand>
#include <qlistview.h>

class InsertCommand : public QUndoCommand
{
public:
    InsertCommand(CustomModelEdit *model, const QString &title, bool type, int row, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    CustomModelEdit *m_model;
    QString m_title;
    bool m_type;
    int m_row;
};

#endif // INSERTCOMMAND_H
