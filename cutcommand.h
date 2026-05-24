#ifndef CUTCOMMAND_H
#define CUTCOMMAND_H

#include <QUndoCommand>
#include <qlistview.h>

class CutCommand : public QUndoCommand
{
public:
    CutCommand(QAbstractItemModel *itemModel, QByteArray &data, const QString &format, const QModelIndexList &selectedindexes);
    void redo() override;
    void undo() override;

private:
    QAbstractItemModel *m_itemModel;
    QModelIndexList m_indexes;
    QByteArray m_data;
    QString m_format;
};

#endif // CUTCOMMAND_H
