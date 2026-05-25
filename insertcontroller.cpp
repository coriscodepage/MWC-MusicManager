#include "insertcontroller.h"
#include <qdebug.h>

InsertController::InsertController(SelectionState *selectionState, QObject *parent)
    : QObject{parent}, m_selectionState(selectionState)
{
    connect(m_selectionState, &SelectionState::songListChanged, this, [this](const ListItem *data) {
        if (!data) return;
        const auto hash = data->getInsertHash();
        revalidateInsert(hash);
    });
}

void InsertController::revalidateInsert(QString hash) {
    const auto listHash = std::find(std::begin(m_inserted), std::end(m_inserted), hash);


    if (listHash != std::end(m_inserted)) {
        int index = std::distance(m_inserted.begin(), listHash);
        switch (index) {
        case Drives::CD1:
            emit insertedChanged(Drives::CD1, true);
            emit insertedChanged(Drives::CD2, false);
            emit insertedChanged(Drives::CD3, false);
            emit insertedChanged(Drives::RADIO, false);
            break;

        case Drives::CD2:
            emit insertedChanged(Drives::CD1, false);
            emit insertedChanged(Drives::CD2, true);
            emit insertedChanged(Drives::CD3, false);
            emit insertedChanged(Drives::RADIO, false);
            break;

        case Drives::CD3:
            emit insertedChanged(Drives::CD1, false);
            emit insertedChanged(Drives::CD2, false);
            emit insertedChanged(Drives::CD3, true);
            emit insertedChanged(Drives::RADIO, false);
            break;

        case Drives::RADIO:
            emit insertedChanged(Drives::CD1, false);
            emit insertedChanged(Drives::CD2, false);
            emit insertedChanged(Drives::CD3, false);
            emit insertedChanged(Drives::RADIO, true);
            break;

        default:
            qWarning() << QString("[InsertController] Unknown state for insert with hash %1").arg(hash);
            break;
        }
    } else {
        emit insertedChanged(Drives::CD1, false);
        emit insertedChanged(Drives::CD2, false);
        emit insertedChanged(Drives::CD3, false);
        emit insertedChanged(Drives::RADIO, false);
    }
    QVector<QString> ins(std::begin(m_inserted), std::end(m_inserted));
    m_selectionState->setInserted(ins);
}

void InsertController::insert(int index) {
    if (index >= 0 && index < 4) {
        const auto currentHash = m_selectionState->currentHash();
        const auto listHash = std::find(std::begin(m_inserted), std::end(m_inserted), currentHash);
        if (listHash != std::end(m_inserted)) {
            int i = std::distance(m_inserted.begin(), listHash);
            if (index == i) return;
            m_inserted[i] = {};
        }
        m_inserted[index] = currentHash;
        revalidateInsert(currentHash);
        qDebug() << QString("[InsertController] Inserting %1 into Drive number %2.").arg(currentHash).arg(index + 1);
    }
}

void InsertController::setInserted(QVector<QString> inserted) {
    if (inserted.count() != 4) return;
    for(auto i = 0; i < 4; i++) {
        m_inserted[i] = inserted[i];
        // revalidateInsert(m_inserted[i]);
    }
    QVector<QString> ins(std::begin(m_inserted), std::end(m_inserted));
    m_selectionState->setInserted(ins);
}

QVector<QString> InsertController::getAllInserted() {
    QVector<QString> ret;
    for(auto i = 0; i < 4; i++) {
        ret.push_back(m_inserted.at(i));
    }
    return ret;
}
