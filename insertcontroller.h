#ifndef INSERTCONTROLLER_H
#define INSERTCONTROLLER_H

#include <QObject>

class InsertController : public QObject
{
    Q_OBJECT
public:
    explicit InsertController(QObject *parent = nullptr);

signals:
};

#endif // INSERTCONTROLLER_H
