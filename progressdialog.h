#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
class ProgressDialog;
}

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = nullptr);
    ~ProgressDialog();

public slots:
    void updateProgress(int percent, const QString &name, const QString &size, const QString &speed, const QString &ETA);

private:
    Ui::ProgressDialog *ui;
};

#endif // PROGRESSDIALOG_H
