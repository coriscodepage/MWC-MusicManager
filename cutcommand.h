#ifndef CUTCOMMAND_H
#define CUTCOMMAND_H

#include "custommodeledit.h"
#include "removecommand.h"
#include <QUndoCommand>
#include <qlistview.h>

class CutCommand : public QUndoCommand
{
public:
    CutCommand(CustomModelEdit *itemModel, QByteArray &data, const QString &format, const QModelIndexList &selectedindexes);
    void redo() override;
    void undo() override;

private:
    CustomModelEdit *m_itemModel;
    QModelIndexList m_indexes;
    QByteArray m_data;
    QString m_format;
    RemoveCommand *m_removeCommand;
};

#endif // CUTCOMMAND_H
