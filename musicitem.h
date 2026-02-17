#ifndef MUSICITEM_H
#define MUSICITEM_H

#include "musicobject.h"
#include <QObject>

class MusicItem
{
public:
    MusicItem(const QString &title, std::shared_ptr<MusicObject> song);
    const QString &title() const;
    QPixmap pixmap() const; // INFO: By value cause the MainWindow needs to resize and I don't want it to bubble up.
    void setTitle(const QString &title);
    void setSong(std::shared_ptr<MusicObject> song);
private:
    QString m_title;
    std::shared_ptr<MusicObject> m_song;
};

#endif // MUSICITEM_H
