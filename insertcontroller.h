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
    // const QVector<QString> &getAllInserted();
    void setInserted(QVector<QString> inserted);

public slots:
    void revalidateInsert(QString hash);
    void insert(int index);

signals:
    void insertedChanged(InsertController::Drives drive, bool inserted);

private:
    QString m_inserted[4];
    SelectionState *m_selectionState;
};

#endif // INSERTCONTROLLER_H
