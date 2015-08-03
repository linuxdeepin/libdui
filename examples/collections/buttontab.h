#ifndef BUTTONTAB_H
#define BUTTONTAB_H

#include <QWidget>
#include <QLabel>

#include "dtextbutton.h"
#include "dimagebutton.h"
#include "dheaderline.h"
#include "darrowbutton.h"
#include "dheaderline.h"
#include "darrowbutton.h"

DUI_USE_NAMESPACE

class ButtonTab : public QLabel
{
    Q_OBJECT
public:
    explicit ButtonTab(QWidget *parent = 0);

public slots:
    void buttonClickTest();
};

#endif // BUTTONTAB_H
