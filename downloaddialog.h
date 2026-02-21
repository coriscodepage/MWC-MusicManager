#ifndef DOWNLOADDIALOG_H
#define DOWNLOADDIALOG_H

#include <QDialog>
#include <qurl.h>

namespace Ui {
class DownloadDialog;
}

class DownloadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DownloadDialog(QWidget *parent = nullptr);
    QUrl url() const;
    bool isPlaylistsAllowed() const;
    ~DownloadDialog();

private:
    Ui::DownloadDialog *ui;
    QUrl m_url;
    bool m_playlistsAllowed;
};

#endif // DOWNLOADDIALOG_H
