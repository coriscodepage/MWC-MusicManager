#ifndef MUSICITEM_H
#define MUSICITEM_H

#include "musicobject.h"
#include <QObject>

class MusicItem
{
public:
    MusicItem() = default;
    MusicItem(const QString &title, std::shared_ptr<MusicObject> song);
    const QString &title() const;
    const QString &titleInternal() const;
    QPixmap pixmap() const; // INFO: By value cause the MainWindow needs to resize and I don't want it to bubble up.
    QFileInfo songPath() const;
    void setTitle(const QString &title);
    void setSong(std::shared_ptr<MusicObject> song);
    const QString &getHash() const;
    void setHash(const QString &hash);

    friend QDataStream &operator<<(QDataStream &out, const MusicItem &item);
    friend QDataStream &operator>>(QDataStream &in, MusicItem &item);
private:
    QString m_title;
    std::shared_ptr<MusicObject> m_song;
    QString m_hash;
};

Q_DECLARE_METATYPE(MusicItem);

#endif // MUSICITEM_H
