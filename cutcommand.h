#ifndef CUTCOMMAND_H
#define CUTCOMMAND_H

#include <QUndoCommand>
#include <qlistview.h>

class CutCommand : public QUndoCommand
{
public:
    CutCommand(QListView *view, QByteArray &data, const QString &format);
    void redo() override;
    void undo() override;

private:
    QListView *m_view;
    QModelIndexList m_indexes;
    QByteArray m_data;
    QString m_format;
};

#endif // CUTCOMMAND_H
