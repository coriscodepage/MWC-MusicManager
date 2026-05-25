#ifndef INSERTCONTROLLER_H
#define INSERTCONTROLLER_H

#include "selectionstate.h"
#include <QObject>

class InsertController : public QObject
{
    Q_OBJECT
public:
    enum Drives {
        CD1 = 0,
        CD2 = 1,
        CD3 = 2,
        RADIO = 3,
    };
    explicit InsertController(SelectionState *selectionState, QObject *parent = nullptr);
    void setInserted(QVector<QString> inserted);
    QVector<QString> getAllInserted();
    bool insertSubdirToGame(const QVector<MusicItem> &songs, int type, const QString &insertHash);

public slots:
    void revalidateInsert(QString hash);
    void insert(int index);

signals:
    void insertedChanged(InsertController::Drives drive, bool inserted);

private:
    std::array<QString, 4> m_inserted;
    SelectionState *m_selectionState;
};

#endif // INSERTCONTROLLER_H
