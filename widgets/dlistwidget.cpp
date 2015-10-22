#include <QWidget>
#include <QVBoxLayout>
#include <QEvent>
#include <QDebug>

#include "dthememanager.h"
#include "dlistwidget.h"
#include "private/dlistwidget_p.h"

DUI_BEGIN_NAMESPACE

DListWidgetPrivate::DListWidgetPrivate(DListWidget *qq):
        DScrollAreaPrivate(qq),
        itemWidth(-1),
        itemHeight(-1),
        layout(new QVBoxLayout),
        checkMode(DListWidget::Radio),
        mainWidget(new QWidget),
        visibleCount(0),
        checkable(false),
        toggleable(false),
        enableVerticalScroll(false)
{

}

DListWidgetPrivate::~DListWidgetPrivate()
{

}

void DListWidgetPrivate::init()
{
    Q_Q(DListWidget);

    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addStretch(1);

    mainWidget->setObjectName("MainWidget");
    mainWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    mainWidget->setLayout(layout);
    mainWidget->setFixedHeight(0);

    q->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
    q->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    q->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    q->setAlignment(Qt::AlignHCenter);
    q->setWidget(mainWidget);
    q->setMaximumHeight(0);
}

DListWidget::DListWidget(CheckMode checkMode, QWidget *parent) :
    DScrollArea(*new DListWidgetPrivate(this), parent)
{
    D_THEME_INIT_WIDGET(DListWidget);

    d_func()->checkMode = checkMode;
    d_func()->init();
}

int DListWidget::addWidget(QWidget *w, Qt::Alignment a)
{
    insertWidget(count(), w, a);

    return count()-1;
}

void DListWidget::addWidgets(const QList<QWidget*> &ws, Qt::Alignment a)
{
    insertWidgets(count(), ws, a);
}

void DListWidget::insertWidget(int index, QWidget *w, Qt::Alignment a)
{
    Q_D(DListWidget);

    if(w==NULL || d->widgetList.contains(w))
        return;

    d->widgetList.insert(index, w);

    if(d->itemWidth>0){
        w->setFixedWidth(d->itemWidth);
    }else{
        d->mainWidget->setFixedWidth(w->width());
    }
    if(d->itemHeight>0){
        w->setFixedHeight(d->itemHeight);
    }

    d->layout->insertWidget(index, w, 0, a);
    if(d->checkable)
        w->installEventFilter(this);

    d->mapVisible[w] = true;

    setHeight(d->mainWidget->height() + w->height() + d->layout->spacing());
    setVisibleCount(d->visibleCount + 1);

    emit countChanged(count());
}

void DListWidget::insertWidgets(int index, const QList<QWidget*> &ws, Qt::Alignment a)
{
    foreach (QWidget *w, ws) {
        insertWidget(index++, w, a);
    }
}

void DListWidget::setItemSize(int w, int h)
{
    Q_D(DListWidget);

    d->itemWidth = w;
    d->itemHeight = h;

    if(d->itemWidth <= 0 && d->itemHeight <= 0)
        return;

    d->mainWidget->setFixedWidth(w);

    foreach (QWidget *w, d->widgetList) {
        if(d->itemWidth>0)
            w->setFixedWidth(d->itemWidth);
        if(d->itemHeight>0){
            w->setFixedHeight(d->itemHeight);
        }
    }
}

void DListWidget::clear(bool isDelete)
{
    Q_D(DListWidget);

    for(int i=0;i<count();++i){
        d->layout->removeItem(d->layout->takeAt(i));
        d->widgetList[i]->removeEventFilter(this);
        d->widgetList[i]->setParent(NULL);
        if(isDelete)
            d->widgetList[i]->deleteLater();
    }

    d->mapVisible.clear();
    d->widgetList.clear();
    d->checkedList.clear();
    d->mainWidget->setFixedHeight(0);

    setMaximumHeight(0);
    setVisibleCount(0);

    emit countChanged(count());
}

void DListWidget::removeWidget(int index, bool isDelete)
{
    Q_D(DListWidget);

    QWidget *w = getWidget(index);

    d->widgetList.removeAt(index);
    d->layout->removeItem(d->layout->takeAt(index));
    d->checkedList.removeOne(index);

    if(d->mapVisible.value(w, false)){
        qDebug() << w->height() << d->mainWidget->height();
        setHeight(d->mainWidget->height() - w->height() - d->layout->spacing());
        setVisibleCount(d->visibleCount -1);
    }
    d->mapVisible.remove(w);

    w->removeEventFilter(this);
    w->setParent(NULL);
    if(isDelete)
        w->deleteLater();

    emit countChanged(count());
}

void DListWidget::showWidget(int index)
{
    Q_D(DListWidget);

    QWidget *w = getWidget(index);

    if(!d->mapVisible.value(w, true)){
        w->show();
        d->mapVisible[w] = true;
        setVisibleCount(d->visibleCount+1);
        setHeight(d->mainWidget->height() + w->height() + d->layout->spacing());
    }
}

void DListWidget::hideWidget(int index)
{
    Q_D(DListWidget);

    QWidget *w = getWidget(index);

    if(d->mapVisible.value(w, false)){
        w->hide();
        d->mapVisible[w] = false;
        setVisibleCount(d->visibleCount-1);
        setHeight(d->mainWidget->height() - w->height() - d->layout->spacing());
    }
}

void DListWidget::setChecked(int index, bool checked)
{
    Q_D(DListWidget);

    if (!d->checkable || (checked && isChecked(index)) || (!checked && !isChecked(index)))
        return;

    if(d->checkMode == Radio && checked){
        int checkedWidget = firstChecked();
        if(checkedWidget>=0){
            getWidget(checkedWidget)->setProperty("checked", false);
            d->checkedList.removeOne(checkedWidget);
        }
    }

    if(index>=0 && index < count()){
        QWidget *w = getWidget(index);
        w->setProperty("checked", checked);
    }

    if(checked){
        d->checkedList << index;
    }else{
        d->checkedList.removeOne(index);
    }

    if(d->checkMode == Radio && checked){
        emit firstCheckedChanged(index);
    }

    emit checkedChanged(index, checked);
}

void DListWidget::setCheckMode(DListWidget::CheckMode checkMode)
{
    Q_D(DListWidget);

    d->checkMode = checkMode;
}

void DListWidget::setCheckable(bool checkable)
{
    Q_D(DListWidget);

    if (d->checkable == checkable)
        return;

    d->checkable = checkable;

    if(checkable){
        foreach (QWidget *w, d->widgetList) {
            w->installEventFilter(this);
        }
    }else{
        foreach (QWidget *w, d->widgetList) {
            w->removeEventFilter(this);
        }
    }
}

void DListWidget::setToggleable(bool enableUncheck)
{
    Q_D(DListWidget);

    if (d->toggleable == enableUncheck)
        return;

    d->toggleable = enableUncheck;
    emit toggleableChanged(enableUncheck);
}

void DListWidget::setEnableVerticalScroll(bool enableVerticalScroll)
{
    Q_D(DListWidget);

    if (d->enableVerticalScroll == enableVerticalScroll)
        return;

    d->enableVerticalScroll = enableVerticalScroll;
    emit enableVerticalScrollChanged(enableVerticalScroll);
    updateGeometry();
}

int DListWidget::count() const
{
    Q_D(const DListWidget);

    return d->widgetList.count();
}

int DListWidget::indexOf(const QWidget* w)
{
    Q_D(DListWidget);

    return d->widgetList.indexOf(const_cast<QWidget*>(w));
}

QWidget *DListWidget::getWidget(int index) const
{
    Q_D(const DListWidget);

    return d->widgetList[index];
}

int DListWidget::firstChecked() const
{
    Q_D(const DListWidget);

    return d->checkedList.count()>0 ? d->checkedList.first() : -1;
}

QList<int> DListWidget::checkedList() const
{
    Q_D(const DListWidget);

    return d->checkedList;
}

bool DListWidget::checkable() const
{
    Q_D(const DListWidget);

    return d->checkable;
}

bool DListWidget::toggleable() const
{
    Q_D(const DListWidget);

    return d->toggleable;
}

bool DListWidget::isChecked(int index) const
{
    Q_D(const DListWidget);

    return d->checkedList.contains(index);
}

int DListWidget::visibleCount() const
{
    Q_D(const DListWidget);

    return d->visibleCount;
}

DListWidget::CheckMode DListWidget::checkMode() const
{
    Q_D(const DListWidget);

    return d->checkMode;
}

bool DListWidget::eventFilter(QObject *obj, QEvent *e)
{
    Q_D(DListWidget);

    if(!d->checkable || e->type() != QEvent::MouseButtonRelease)
        return false;

    QWidget *w = qobject_cast<QWidget*>(obj);

    if(w){
        int index = indexOf(w);
        if(index>=0){
            if(d->toggleable)
                setChecked(index, !isChecked(index));
            else
                setChecked(index, true);

            emit clicked(index);
        }
    }

    return false;
}

QSize DListWidget::sizeHint() const
{
    Q_D(const DListWidget);

    QSize size;
    size.setWidth(d->mainWidget->width());
    if(d->enableVerticalScroll)
        size.setHeight(qMin(maximumHeight(), d->mainWidget->height()));
    else
        size.setHeight(d->mainWidget->height());

    return size;
}

void DListWidget::setVisibleCount(int count)
{
    Q_D(DListWidget);

    if(d->visibleCount == count)
        return;

    d->visibleCount = count;

    emit visibleCountChanged(count);
}

void DListWidget::setHeight(int height)
{
    Q_D(DListWidget);

    d->mainWidget->setFixedHeight(height);
    if(!d->enableVerticalScroll){
        setFixedHeight(d->mainWidget->height());
    }else{
        resize(d->mainWidget->size());
    }
    updateGeometry();
}

QList<QWidget*> DListWidget::widgetList() const
{
    Q_D(const DListWidget);

    return d->widgetList;
}

QSize DListWidget::itemSize() const
{
    Q_D(const DListWidget);

    return QSize(d->itemWidth, d->itemHeight);
}

bool DListWidget::enableVerticalScroll() const
{
    Q_D(const DListWidget);

    return d->enableVerticalScroll;
}

DListWidget::DListWidget(DListWidgetPrivate &dd, QWidget *parent):
    DScrollArea(dd, parent)
{
    D_THEME_INIT_WIDGET(DListWidget);

    d_func()->init();
}

DUI_END_NAMESPACE