#include "dsegmentedcontrol.h"
#include <QDebug>
#include <QPainter>
#include <QPaintEvent>
#include <QApplication>
#include <QStyleOption>

DUI_BEGIN_NAMESPACE

DSegmentedHighlight::DSegmentedHighlight(QWidget *parent) :
    QWidget(parent)
{
}


void DSegmentedHighlight::paintEvent(QPaintEvent *e)
{
    QStyleOption opt;

    opt.init(this);

    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(e);
}

DSegmentedControl::DSegmentedControl(QWidget *parent) :
    QWidget(parent),
    m_highlight(new DSegmentedHighlight(this)),
    m_hLayout(new QHBoxLayout(this)),
    m_currentIndex(-1),
    m_highlightMoveAnimation(new QPropertyAnimation(m_highlight, "geometry", this))
{
    m_hLayout->setSpacing(0);
    m_hLayout->setMargin(0);

    m_hLayout->setObjectName("TabBar");
    m_highlight->setObjectName("Highlight");
    m_highlightMoveAnimation->setDuration(50);
    m_highlightMoveAnimation->setEasingCurve(QEasingCurve::InCubic);
}

int DSegmentedControl::count() const
{
    return m_hLayout->count();
}

const DSegmentedHighlight *DSegmentedControl::highlight() const
{
    return m_highlight;
}

int DSegmentedControl::currentIndex() const
{
    return m_currentIndex;
}

QToolButton *DSegmentedControl::at(int index) const
{
    return m_tabList[index];
}

QString DSegmentedControl::getText(int index) const
{
    const QToolButton *button = at(index);

    if(button){
        return button->text();
    }

    return "";
}

QIcon DSegmentedControl::getIcon(int index) const
{
    return at(index)->icon();
}

int DSegmentedControl::animationDuration() const
{
    return m_highlightMoveAnimation->duration();
}

QEasingCurve::Type DSegmentedControl::animationType() const
{
    return m_highlightMoveAnimation->easingCurve().type();
}

int DSegmentedControl::addSegmented(const QString &title)
{
    insertSegmented(m_hLayout->count(), title);

    return m_hLayout->count()-1;
}

int DSegmentedControl::addSegmented(const QIcon &icon, const QString &title)
{
    insertSegmented(m_hLayout->count(), icon, title);

    return m_hLayout->count()-1;
}

void DSegmentedControl::insertSegmented(int index, const QString &title)
{
    insertSegmented(index, QIcon(), title);
}

void DSegmentedControl::insertSegmented(int index, const QIcon &icon, const QString &title)
{
    QToolButton *button = new QToolButton();

    m_tabList.insert(index, button);

    button->setObjectName("Segmented"+QString::number(index));
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    button->setText(title);
    button->setIcon(icon);

    connect(button, &QToolButton::clicked, this, &DSegmentedControl::buttonClicked);
    m_hLayout->insertWidget(index, button);

    if(m_currentIndex == -1){
        setCurrentIndex(0);
    }

    button->installEventFilter(this);
}

void DSegmentedControl::setCurrentIndex(int currentIndex)
{
    if(currentIndex == m_currentIndex)
        return;

    m_currentIndex = currentIndex;

    foreach (QToolButton *button, m_tabList) {
        button->setEnabled(true);
    }

    at(currentIndex)->setFocus();
    at(currentIndex)->setEnabled(false);

    updateHighlightGeometry();

    emit currentChanged(currentIndex);
}

void DSegmentedControl::setText(int index, const QString &title)
{
    at(index)->setText(title);
}

void DSegmentedControl::setIcon(int index, const QIcon &icon)
{
    at(index)->setIcon(icon);
}

void DSegmentedControl::setAnimationDuration(int animationDuration)
{
    m_highlightMoveAnimation->setDuration(animationDuration);
}

void DSegmentedControl::setAnimationType(QEasingCurve::Type animationType)
{
    m_highlightMoveAnimation->setEasingCurve(animationType);
}

void DSegmentedControl::paintEvent(QPaintEvent *e)
{
    QStyleOption opt;

    opt.init(this);

    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(e);
}

bool DSegmentedControl::eventFilter(QObject *obj, QEvent *e)
{
    if(obj == at(m_currentIndex)){
        if(e->type() == QEvent::Resize){
            updateHighlightGeometry();
        }
    }

    return false;
}

void DSegmentedControl::updateHighlightGeometry()
{
    if(m_currentIndex>=0){
        m_highlightMoveAnimation->setStartValue(m_highlight->geometry());
        m_highlightMoveAnimation->setEndValue(at(m_currentIndex)->geometry());
        m_highlightMoveAnimation->start();
    }
}

void DSegmentedControl::buttonClicked()
{
    int i = m_tabList.indexOf(qobject_cast<QToolButton*>(sender()));

    if(i>=0){
        setCurrentIndex(i);
    }
}

DUI_END_NAMESPACE
