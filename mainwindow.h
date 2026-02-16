#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "primarylistmodel.h"
#include "secondarylistmodel.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_addCD_clicked();
    void handlePrimaryListSelectionChanged(const QModelIndex &index);

    void on_newItem_clicked();

    void on_deleteList_clicked();

private:
    Ui::MainWindow *ui;
    PrimaryListModel *m_primarymodel;
    SecondaryListModel *m_secondarymodel;
};
#endif // MAINWINDOW_H
