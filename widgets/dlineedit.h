#ifndef DLINEEDIT_H
#define DLINEEDIT_H

#include <QLineEdit>

#include "libdui_global.h"

DUI_BEGIN_NAMESPACE

class LIBDUISHARED_EXPORT DLineEdit : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY(bool alert READ isAlert NOTIFY alertChanged)

public:
    DLineEdit(QWidget *parent = 0);

    void setAlert(bool isAlert);
    inline bool isAlert() const {return m_isAlert;}

signals:
    void alertChanged();

private:
    bool m_isAlert;
};

DUI_END_NAMESPACE

#endif // DLINEEDIT_H
