#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H


#include "musicitem.h"
class GameManager
{
public:
    enum Drives {
        CD1 = 0,
        CD2 = 1,
        CD3 = 2,
        RADIO = 3,
    };
    GameManager(const QDir &gamePath);
    void insertSubdirToGame(const QVector<MusicItem> &songs, const Drives &type, const QString &insertHash);
    const QString &getInsertedHash(const Drives &drive);
    void setInserted(QVector<QString> &list);
    const QVector<QString> &getAllInserted();


private:
    QDir m_gamePath;
    QVector<QString> m_inserted;
};

#endif // GAMEMANAGER_H
