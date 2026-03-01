#ifndef INSERTPRIMARYCOMMAND_H
#define INSERTPRIMARYCOMMAND_H

#include "primarylistmodel.h"
#include <QUndoCommand>
#include <qlistview.h>

class InsertPrimaryCommand : public QUndoCommand
{
public:
    InsertPrimaryCommand(QListView *view, PrimaryListModel *model, const QString &title, bool type, int row, bool edit = true, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    QListView *m_view;
    PrimaryListModel *m_model;
    QString m_title;
    bool m_type;
    int m_row;
    bool m_edit;
};

#endif // INSERTPRIMARYCOMMAND_H
