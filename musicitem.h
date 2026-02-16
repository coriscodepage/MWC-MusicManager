#ifndef MUSICITEM_H
#define MUSICITEM_H

#include <QObject>

class MusicItem
{
public:
    MusicItem(const QString &title);
    const QString &title() const;
    void setTitle(const QString &title);

private:
    QString m_title;
};

#endif // MUSICITEM_H
