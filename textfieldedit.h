#ifndef TEXTFIELDEDIT_H
#define TEXTFIELDEDIT_H

#include <qabstractitemmodel.h>
#include <qcontainerfwd.h>
class TextFieldEdit
{
public:
    TextFieldEdit() = default;
    virtual void setField(int field, const QString &value, const QModelIndex &index) = 0;
    virtual QString getField(int field, const QModelIndex &index) = 0;
    virtual ~TextFieldEdit() = default;
};

#endif // TEXTFIELDEDIT_H
