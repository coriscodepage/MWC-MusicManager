#ifndef TEXTFIELDEDIT_H
#define TEXTFIELDEDIT_H

#include <qabstractitemmodel.h>
#include <qcontainerfwd.h>
class CustomModelEdit
{
public:
    CustomModelEdit() = default;
    virtual void setField(int field, const QString &value, const QModelIndex &index) = 0;
    virtual QString getField(int field, const QModelIndex &index) const = 0;
    virtual void removeAt(int row) = 0;
    virtual void insertEmptyAt(int row, const QString &name, bool type) = 0;
    virtual void moveInternal(const QVector<QVariant> &movingItems, int sourceRow, int count, int destinationChild) = 0;
    virtual void beginMacro(const QString &name) = 0;
    virtual void endMacro() = 0;
    virtual const QVariant getSelf(QModelIndexList indexes) const = 0;
    virtual const QVector<QVariant> getDependent() const = 0;
    virtual void restoreEntry(const QVariant &selfData, const QVector<QVariant> &childData, QModelIndexList indexes) = 0;
    virtual ~CustomModelEdit() = default;
};

#endif // TEXTFIELDEDIT_H
