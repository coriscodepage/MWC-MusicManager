#ifndef REMOVECOMMAND_H
#define REMOVECOMMAND_H

#include "secondarylistmodel.h"
#include <QUndoCommand>
#include <qlistview.h>

class RemoveCommand : public QUndoCommand
{
public:
    RemoveCommand(QListView *view, int row, SecondaryListModel *secondaryModel = nullptr, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    QListView *m_view;
    SecondaryListModel *m_secondaryModel;
    QByteArray m_data;
    QString m_format;
    int m_row;
};

#endif // REMOVECOMMAND_H
