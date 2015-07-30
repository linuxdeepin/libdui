#ifndef LINETAB_H
#define LINETAB_H

#include <QWidget>
#include <QPushButton>

#include "dheaderline.h"
#include "dbaseline.h"
#include "dseparatorhorizontal.h"
#include "dseparatorvertical.h"
#include "dbaseexpand.h"
#include "darrowlineexpand.h"

DUI_USE_NAMESPACE

class LineTab : public QLabel
{
    Q_OBJECT
public:
    explicit LineTab(QWidget *parent = 0);

signals:

public slots:
};

#endif // LINETAB_H
