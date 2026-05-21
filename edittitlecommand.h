#ifndef EDITTITLECOMMAND_H
#define EDITTITLECOMMAND_H

#include "custommodeledit.h"
#include <QUndoCommand>

class EditTitleCommand : public QUndoCommand
{
public:
    EditTitleCommand(CustomModelEdit *model, int field, const QString &value, const QModelIndex &index);
    void undo() override;
    void redo() override;

private:
    CustomModelEdit *m_model;
    int m_field;
    QString m_value;
    QString m_oldValue;
    QModelIndex m_index;
};

#endif // EDITTITLECOMMAND_H
