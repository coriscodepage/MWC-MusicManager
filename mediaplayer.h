#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include "musicitem.h"
#include "selectionstate.h"
#include <QObject>
#include <qmediaplayer.h>

class MediaPlayer : public QObject
{
    Q_OBJECT

public:
    explicit MediaPlayer(const SelectionState *selectionState, QObject *parent = nullptr);

public slots:
    void changeSong();
    void play();
    void pause();
    void stop();
    void checkIfDeleted(const QString &hash);
signals:
    void labelChanged(const QString& label);
    void playState(bool state);
    void pauseState(bool state);
    void stopState(bool state);
    void positionChanged(qint64 duration, qint64 position);

private:
    QMediaPlayer *m_mediaPlayer;
    const SelectionState *m_selectionState;
    const MusicItem *m_currentSong;
    void clear();
};

#endif // MEDIAPLAYER_H
