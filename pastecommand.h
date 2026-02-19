#ifndef PASTECOMMAND_H
#define PASTECOMMAND_H

#include <QUndoCommand>
#include <qlistview.h>

class PasteCommand : public QUndoCommand
{
public:
    PasteCommand(QListView *view, const QMimeData *data, int m_row = -1);
    void undo() override;
    void redo() override;
private:
    QListView *m_view;
    const QMimeData *m_data;
    const QMimeData *m_previousData;
    int m_row;

};

#endif // PASTECOMMAND_H
