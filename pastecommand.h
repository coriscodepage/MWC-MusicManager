#ifndef PASTECOMMAND_H
#define PASTECOMMAND_H

#include <QUndoCommand>
#include <qlistview.h>

class PasteCommand : public QUndoCommand
{
public:
    PasteCommand(QListView *view, QByteArray &data, const QString &format, int m_row = -1);
    void undo() override;
    void redo() override;
private:
    QListView *m_view;
    QByteArray m_data;
    QByteArray m_oldData;
    int m_row;
    int m_count;
    QString m_format;
};

#endif // PASTECOMMAND_H
