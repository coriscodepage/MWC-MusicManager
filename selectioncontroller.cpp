#include "selectioncontroller.h"
#include <qpixmap.h>

SelectionController::SelectionController(PrimaryListModel *listModel, SecondaryListModel *songModel, SelectionState *state, QObject *parent)
    : QObject{parent}, m_listModel(listModel), m_songModel(songModel), m_selectionState(state)
{}

void SelectionController::handlePrimaryListSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    Q_UNUSED(deselected);
    if (selected.isEmpty()) {
        qDebug() << "[SelectionController] Empty primary selection.";
        m_selectionState->updateList(nullptr);
        // emit SongListChanged({}, -1);
    }

    if (selected.length() == 1) {
        qDebug() << QString("[SelectionController] Primary: {%1}").arg(selected.indexes().constFirst().row());
        QModelIndex index = selected.indexes().constFirst();
        ListItem &data = m_listModel->getItem(index);
        m_songModel->setSource(&data);
        m_selectionState->updateList(&data);
    } else {
        qDebug() << "[SelectionController] Multi selection.";
    }
}

void SelectionController::handleSecondaryListSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    Q_UNUSED(deselected);
    if (selected.isEmpty()) {
        qDebug() << "[SelectionController] Empty secondary selection.";
        m_selectionState->updateState(selected, {});
    }
    if (selected.length() == 1) {
        qDebug() << QString("[SelectionController] Secondary: {%1}").arg(selected.indexes().constFirst().row());
    } else {
        qDebug() << "[SelectionController] Multi selection.";
    }

    const auto indecies = selected.indexes();
    QVector<const MusicItem *> resolved;
    for (const auto &index : indecies) {
        const auto song = m_songModel->getSong(index.row());
        resolved.push_back(song);
    }

    m_selectionState->updateState(selected, resolved);

}

