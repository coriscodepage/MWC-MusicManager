#ifndef MUSICLIST_H
#define MUSICLIST_H

#include "musicitem.h"
class MusicList
{
public:
    MusicList();

private:
    QVector<MusicItem> m_items;
};

#endif // MUSICLIST_H
