#ifndef CHILDLISTVIEW_H
#define CHILDLISTVIEW_H

#include "custommodeledit.h"
#include <QListView>
#include <QObject>

class ChildListView : public QListView
{
    Q_OBJECT
public:
    using QListView::QListView;
    void setForceCopy(bool state);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    bool m_forcedCopy = false;

signals:
    void forceCopy(bool state);
};

struct DropMacroGuard {
    CustomModelEdit *m;
    DropMacroGuard(CustomModelEdit *m, const QString &name) : m(m) { if (m) m->beginMacro(name); }
    ~DropMacroGuard()                    { if (m) m->endMacro(); }
};

#endif // CHILDLISTVIEW_H
