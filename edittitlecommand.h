#ifndef EDITTITLECOMMAND_H
#define EDITTITLECOMMAND_H

#include "textfieldedit.h"
#include <QUndoCommand>

class EditTitleCommand : public QUndoCommand
{
public:
    EditTitleCommand(TextFieldEdit *model, int field, const QString &value, const QModelIndex &index);
    void undo() override;
    void redo() override;

private:
    TextFieldEdit *m_model;
    int m_field;
    QString m_value;
    QString m_oldValue;
    QModelIndex m_index;
};

#endif // EDITTITLECOMMAND_H
