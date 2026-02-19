#ifndef TARGETLISTVIEW_H
#define TARGETLISTVIEW_H

#include <QListView>

class TargetListView : public QListView
{
    Q_OBJECT

public:
    using QListView::QListView;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

#endif // TARGETLISTVIEW_H
