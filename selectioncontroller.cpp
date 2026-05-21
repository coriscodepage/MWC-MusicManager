#include "selectioncontroller.h"
#include <qpixmap.h>

SelectionController::SelectionController(PrimaryListModel *listModel, SecondaryListModel *songModel, QObject *parent)
    : QObject{parent}, m_listModel(listModel), m_songModel(songModel)
{}

void SelectionController::handlePrimaryListSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    Q_UNUSED(deselected);
    if (selected.isEmpty()) {
        qDebug() << "[SelectionController] Empty selection.";
        emit songListState(false);
    }

    if (selected.length() == 1) {
        qDebug() << QString("[SelectionController] Primary: {%1}").arg(selected.indexes().constFirst().row());
        QModelIndex index = selected.indexes().constFirst();
        ListItem &data = m_listModel->getItem(index);
        m_songModel->setSource(&data);
        bool state = m_listModel->rowCount() > 0;
        emit songListState(true);
        emit songListMembersPresent(state);
        emit songSelected(MusicInfo(), resolveThumbnail(""));
    } else {
        qDebug() << "[SelectionController] Multi selection.";
    }
}

void SelectionController::handleSecondaryListSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected) {
    Q_UNUSED(deselected);
    if (selected.isEmpty()) {
        qDebug() << "[SelectionController] Empty selection.";
        emit songListState(false);
    }

    if (selected.length() == 1) {
        qDebug() << QString("[SelectionController] Secondary: {%1}").arg(selected.indexes().constFirst().row());
        QModelIndex index = selected.indexes().constFirst();
        const auto song = m_songModel->getSong(index.row());
        const auto info = song->musicInfo();
        auto pixmap = resolveThumbnail(info.thumbnailPath);
        emit songSelected(info, pixmap);

    } else {
        qDebug() << "[SelectionController] Multi selection.";
    }
}

QPixmap SelectionController::resolveThumbnail(const QString &path) {
    static const QPixmap empty(":/defaults/no-image.webp");
    QPixmap pixmap = empty;
    if (!path.isEmpty()) {
        pixmap = QPixmap(path);
    }
    return pixmap;
}