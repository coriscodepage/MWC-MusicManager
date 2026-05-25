#include "selectionstate.h"
#include <qpixmap.h>

SelectionState::SelectionState(QObject *parent)
    : QObject{parent} {}

void SelectionState::updateState(QItemSelection currentSelection, QVector<const MusicItem *> resolvedSongs) {
    m_currentSelection = currentSelection;
    m_resolvedSongs = resolvedSongs;
    if (m_resolvedSongs.count() == 1) {
        // qDebug() << "[SelectionState] Single selection";
        m_currentInfo = m_resolvedSongs.constFirst()->musicInfo();
        emit songSelected(&m_currentInfo, resolveThumbnail(m_currentInfo.thumbnailPath));
    } else if (m_resolvedSongs.count() == 0) {
        // qDebug() << "[SelectionState] No selection";
        m_currentInfo = {};
        emit songSelected(nullptr, resolveThumbnail({}));
    } else {
        // qDebug() << "[SelectionState] Multi selection";
    }
}


void SelectionState::revalidate() {
    if (m_resolvedSongs.count() == 1) {
        // qDebug() << "[SelectionState] Revalidated single selection";
        m_currentInfo = m_resolvedSongs.constFirst()->musicInfo();
        emit songSelected(&m_currentInfo, resolveThumbnail(m_currentInfo.thumbnailPath));
    } else if (m_resolvedSongs.count() == 0) {
        // qDebug() << "[SelectionState] Revalidated no selection";
        // m_currentInfo = {};
        emit songSelected(nullptr, resolveThumbnail({}));
    } else {
        // qDebug() << "[SelectionState] Revalidated multi selection";
    }
}

const QItemSelection &SelectionState::currentSelection() const {
    return m_currentSelection;
}
const MusicItem * SelectionState::currentSong() const {
    if (m_resolvedSongs.count() == 1) {
    return m_resolvedSongs.constFirst();
    } else {
        return nullptr;
    }
}
const MusicInfo &SelectionState::currentInfo() const {
    return m_currentInfo;
}

const QString &SelectionState::currentHash() const {
    return m_currentListItem->getInsertHash();
}

void SelectionState::updateList(const ListItem *data) {
    if (data) {
    m_currentListItem = data;
    } else {
        m_currentListItem = nullptr;
    }
    emit songSelected({}, resolveThumbnail({}));
    emit songListChanged(data);
}

QPixmap SelectionState::resolveThumbnail(const QString &path) {
    static const QPixmap empty(":/defaults/no-image.webp");
    QPixmap pixmap = empty;
    if (!path.isEmpty()) {
        pixmap = QPixmap(path);
    }
    return pixmap;
}

 const QVector<QString> &SelectionState::inserted() const {
    return m_inserted;
}

void SelectionState::setInserted(QVector<QString> inserted) {
    m_inserted = inserted;
}