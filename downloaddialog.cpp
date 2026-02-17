#include "downloaddialog.h"
#include "ui_downloaddialog.h"

DownloadDialog::DownloadDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DownloadDialog)
{
    ui->setupUi(this);
}

QUrl DownloadDialog::url() const {
    return ui->lineEdit->text();
}

DownloadDialog::~DownloadDialog()
{
    delete ui;
}
