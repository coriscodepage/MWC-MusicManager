#include "mediaplayer.h"
#include <qaudiooutput.h>
#include <qdebug.h>

MediaPlayer::MediaPlayer(const SelectionState *selectionState, QObject *parent)
    : QObject{parent}, m_selectionState(selectionState), m_currentSong(nullptr)
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
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
        emit positionChanged(m_mediaPlayer->duration(), position);
    });
    connect(m_mediaPlayer, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error error, const QString &errorString) {
        if (error == QMediaPlayer::NoError)
            return;
        qWarning() << "[MediaPlayer] Playback error:" << error << errorString;
        clear();
    });
}

void MediaPlayer::changeSong() {
    const auto song = m_selectionState->currentSong();
    if (!song) return;
    m_currentSong = song;
    const auto path = m_currentSong->songPath();
    const auto title = m_currentSong->title();
    if (path.isEmpty()) return;
    m_mediaPlayer->setSource(QUrl::fromLocalFile(path));
    emit labelChanged(title);
    play();
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

void MediaPlayer::checkIfDeleted(const QString &hash) {
    if (!m_currentSong) return;
    if (m_currentSong->getHash() != hash) return;
    clear();
}

void MediaPlayer::clear() {
    m_mediaPlayer->stop();
    m_mediaPlayer->setSource({});
    m_currentSong = nullptr;
    emit stopState(false);
    emit pauseState(false);
    emit playState(false);
    emit labelChanged({});
    emit positionChanged(-1, -1);
}
