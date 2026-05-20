#include "mediaplayer.h"
#include <qaudiooutput.h>

MediaPlayer::MediaPlayer(QObject *parent)
    : QObject{parent}, m_currentSong(nullptr)
{
    m_mediaPlayer = new QMediaPlayer(this);
    QAudioOutput* audioOutput = new QAudioOutput(this);
    audioOutput->setVolume(50);
    m_mediaPlayer->setAudioOutput(audioOutput);
}

void MediaPlayer::changeSong(const MusicItem *song) {
    if (!song) return;
    m_currentSong = song;
}
void MediaPlayer::play() {
    if (m_currentSong == nullptr) return;
    const auto path = m_currentSong->songPath();
    const auto title = m_currentSong->title();
    if (path.isEmpty()) return;
    m_mediaPlayer->setSource(path);
    m_mediaPlayer->play();
    emit labelChanged(title);
}
void MediaPlayer::pause() {
    m_mediaPlayer->pause();
}
void MediaPlayer::stop() {
    m_mediaPlayer->stop();
}