#ifndef SELECTIONCONTROLLER_H
#define SELECTIONCONTROLLER_H

#include "primarylistmodel.h"
#include "secondarylistmodel.h"
#include <QObject>
#include <qitemselectionmodel.h>

class SelectionController : public QObject
{
    Q_OBJECT
public:
    explicit SelectionController(PrimaryListModel *listModel, SecondaryListModel *songModel, QObject *parent = nullptr);

public slots:
    void handlePrimaryListSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void handleSecondaryListSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    PrimaryListModel *m_listModel;
    SecondaryListModel *m_songModel;
    QPixmap resolveThumbnail(const QString &path);

signals:
    void songListState(bool state);
    void songListMembersPresent(bool state);
    void songSelected(MusicInfo info, QPixmap pixmap);
};

#endif // SELECTIONCONTROLLER_H
