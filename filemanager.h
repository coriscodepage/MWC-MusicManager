#ifndef FILEMANAGER_H
#define FILEMANAGER_H

// Source - https://stackoverflow.com/a/1008289
// Posted by Loki Astari, modified by community. See post 'Timeline' for change history
// Retrieved 2026-05-22, License - CC BY-SA 4.0

#include "insertcontroller.h"
#include "musicitem.h"
class FileManager
{
public:
    static FileManager& getInstance()
    {
        static FileManager    instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
private:
    FileManager() {}                    // Constructor? (the {} brackets) are needed here.

    // C++ 11
    // =======
    // We can use the better technique of deleting the methods
    // we don't want.

    QString m_gamePath;
    QString m_musicPath;
    QString m_appPath;
    QString m_appSaveName;

public:
    FileManager(FileManager const&)     = delete;
    void operator=(FileManager const&)  = delete;
    // Note: Scott Meyers mentions in his Effective Modern
    //       C++ book, that deleted functions should generally
    //       be public as it results in better error messages
    //       due to the compilers behavior to check accessibility
    //       before deleted status

    void copySongsToDrive(const QVector<MusicItem> &songs, const InsertController::Drives drive);
    QByteArray loadSaveFile();
    void setGamePath(QString path);
    void setMusicPath(QString path);
    void setAppPath(QString path);
    void setSaveName(QString name);
    const QString &getGamePath() const;
    const QString &getMusicPath() const;
    const QString &getAppPath() const;
    const QString &getSaveName() const;

};

#endif // FILEMANAGER_H
