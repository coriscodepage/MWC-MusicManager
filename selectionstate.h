#ifndef SELECTIONSTATE_H
#define SELECTIONSTATE_H

#include "listitem.h"
#include "musicitem.h"
#include "musicobject.h"
#include <qitemselectionmodel.h>
class SelectionState: public QObject
{
    Q_OBJECT
public:
    explicit SelectionState(QObject *parent = nullptr);
    void updateState(QItemSelection currentSelection, QVector<const MusicItem *> resolvedSongs);
    void updateList(const ListItem *data);
    void setInserted(QVector<QString> inserted);
    const QItemSelection &currentSelection() const;
    const MusicItem *currentSong() const;
    const MusicInfo &currentInfo() const;
    const QString &currentHash() const;
    const QVector<QString> &inserted() const;

private:
    MusicInfo m_currentInfo;
    QItemSelection m_currentSelection;
    QVector<const MusicItem *> m_resolvedSongs = {};
    const ListItem *m_currentListItem;
    QPixmap resolveThumbnail(const QString &path);
    QVector<QString> m_inserted;

signals:
    void songSelected(MusicInfo *info, QPixmap pixmap);
    void songListChanged(const ListItem *data);

};

#endif // SELECTIONSTATE_H
