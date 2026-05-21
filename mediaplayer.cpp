#include "mediaplayer.h"
#include <qaudiooutput.h>

MediaPlayer::MediaPlayer(QObject *parent)
    : QObject{parent}, m_currentSong(nullptr)
{
    m_mediaPlayer = new QMediaPlayer(this);
    QAudioOutput* audioOutput = new QAudioOutput(this);
    audioOutput->setVolume(75);
    m_mediaPlayer->setAudioOutput(audioOutput);
    connect(m_mediaPlayer, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state) {
        if (state == QMediaPlayer::StoppedState) {
            emit stopState(false);
            emit pauseState(false);
            emit playState(true);
        }
    });
}

void MediaPlayer::changeSong(const MusicItem *song) {
    if (!song) return;
    m_currentSong = song;
    const auto path = m_currentSong->songPath();
    const auto title = m_currentSong->title();
    if (path.isEmpty()) return;
    m_mediaPlayer->setSource(QUrl::fromLocalFile(path));
    emit labelChanged(title);
}
void MediaPlayer::play() {
    if (m_currentSong == nullptr) return;
    m_mediaPlayer->play();
    emit stopState(true);
    emit pauseState(true);
    emit playState(false);
}
void MediaPlayer::pause() {
    m_mediaPlayer->pause();
    emit stopState(true);
    emit pauseState(false);
    emit playState(true);
}
void MediaPlayer::stop() {
    m_mediaPlayer->stop();
}