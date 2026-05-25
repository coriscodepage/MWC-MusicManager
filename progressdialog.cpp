#include "progressdialog.h"
#include "ui_progressdialog.h"

ProgressDialog::ProgressDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

void ProgressDialog::updateProgress(int percent, const QString &name, const QString &size, const QString &speed, const QString &ETA) {
    if (percent >= 0) {
        ui->progressBar->setMinimum(0);
        ui->progressBar->setMaximum(100);
        ui->progressBar->setValue(percent);
    }
    ui->nameLabel->setText(name.isEmpty() ? tr("Downloading media") : name);
    ui->speedLabel->setText(speed);
    ui->ETALabel->setText(ETA);
}