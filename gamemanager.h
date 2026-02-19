#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H


#include "musicitem.h"
class GameManager
{
public:
    GameManager(const QDir &gamePath);
    void insertSubdirToGame(const QVector<MusicItem> &songs, const QString &subDir);

private:
    QDir m_gamePath;
};

#endif // GAMEMANAGER_H
