#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include "musicitem.h"
#include <QObject>
#include <qmediaplayer.h>

class MediaPlayer : public QObject
{
    Q_OBJECT

public:
    explicit MediaPlayer(QObject *parent = nullptr);

private:
    QMediaPlayer *m_mediaPlayer;
    const MusicItem *m_currentSong;

public slots:
    void changeSong(const MusicItem *song);
    void play();
    void pause();
    void stop();
signals:
    void labelChanged(const QString& label);
};

#endif // MEDIAPLAYER_H
