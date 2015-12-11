#include <QPushButton>
#include <QLabel>
#include <QButtonGroup>
#include <QDebug>
#include <QCloseEvent>
#include <QApplication>
#include <QSpacerItem>
#include <QDesktopWidget>
#include <QScreen>

#include "private/ddialog_p.h"

#include "dialog_constants.h"
#include "ddialog.h"
#include "dthememanager.h"
#include "dboxwidget.h"

DUI_BEGIN_NAMESPACE

DDialogPrivate::DDialogPrivate(DDialog *qq) :
    DAbstractDialogPrivate(qq)
{

}

void DDialogPrivate::init()
{
    D_Q(DDialog);

    buttonLayout = new QHBoxLayout;

    buttonLayout->setMargin(0);
    buttonLayout->setSpacing(0);
    buttonLayout->setContentsMargins(DIALOG::BUTTON_LAYOUT_LEFT_MARGIN,
                                     DIALOG::BUTTON_LAYOUT_TOP_MARGIN,
                                     DIALOG::BUTTON_LAYOUT_RIGHT_MARGIN,
                                     DIALOG::BUTTON_LAYOUT_BOTTOM_MARGIN);

    closeButton = new QPushButton(q);

    closeButton->setObjectName("CloseButton");
    closeButton->setFixedSize(DIALOG::CLOSE_BUTTON_WIDTH, DIALOG::CLOSE_BUTTON_WIDTH);
    closeButton->setAttribute(Qt::WA_NoMousePropagation);

    iconLabel = new QLabel;
    iconLabel->hide();

    messageLabel = new QLabel;
    messageLabel->setObjectName("MessageLabel");
    messageLabel->hide();

    titleLabel = new QLabel;
    titleLabel->setObjectName("TitleLabel");
    titleLabel->hide();

    contentLayout = new QVBoxLayout;

    contentLayout->setMargin(0);
    contentLayout->setSpacing(0);
    contentLayout->addWidget(titleLabel);
    contentLayout->addWidget(messageLabel);

    iconLayout = new QHBoxLayout;

    iconLayout->setContentsMargins(DIALOG::ICON_LAYOUT_LEFT_MARGIN,
                                   DIALOG::ICON_LAYOUT_TOP_MARGIN,
                                   DIALOG::ICON_LAYOUT_RIGHT_MARGIN,
                                   DIALOG::ICON_LAYOUT_BOTTOM_MARGIN);
    iconLayout->addWidget(iconLabel);
    iconLayout->addLayout(contentLayout);

    QVBoxLayout *main_layout = new QVBoxLayout;

    main_layout->setMargin(0);
    main_layout->addWidget(closeButton, 0, Qt::AlignRight);
    main_layout->addLayout(iconLayout);
    main_layout->addLayout(buttonLayout);

    QObject::connect(closeButton, SIGNAL(clicked()), q, SLOT(close()));
    QObject::connect(q, SIGNAL(sizeChanged(QSize)), q, SLOT(_q_updateLabelMaxWidth()));

    q->setLayout(main_layout);
}

const QScreen *DDialogPrivate::getScreen() const
{
    D_QC(DDialog);

    const QScreen *screen = qApp->screens()[qApp->desktop()->screenNumber(q)];

    if(screen)
        return screen;

    screen = qApp->screens()[qApp->desktop()->screenNumber(QCursor::pos())];

    return screen;
}

void DDialogPrivate::_q_onButtonClicked()
{
    D_Q(DDialog);

    QAbstractButton *button = qobject_cast<QAbstractButton*>(q->sender());

    if(button) {
        clickedButtonIndex = buttonList.indexOf(button);
        q->buttonClicked(clickedButtonIndex, button->text());

        if(onButtonClickedClose)
            q->done(clickedButtonIndex);
    }
}

void DDialogPrivate::_q_updateLabelMaxWidth()
{
    D_Q(DDialog);

    int labelMaxWidth = q->maximumWidth() - titleLabel->x() - DIALOG::CLOSE_BUTTON_WIDTH;

    QFontMetrics fm = titleLabel->fontMetrics();

    if (fm.width(title) > labelMaxWidth){
        QString text = fm.elidedText(title, Qt::ElideRight, labelMaxWidth);

        titleLabel->setText(text);
    }

    fm = messageLabel->fontMetrics();

    if (fm.width(message) > labelMaxWidth){
        QString text = fm.elidedText(message, Qt::ElideRight, labelMaxWidth);

        messageLabel->setText(text);
    }
}

DDialog::DDialog(QWidget *parent) :
    DAbstractDialog(*new DDialogPrivate(this), parent)
{
    D_THEME_INIT_WIDGET(dialogs/DDialog);

    d_func()->init();
}

DDialog::DDialog(const QString &title, const QString &message, QWidget *parent) :
    DAbstractDialog(*new DDialogPrivate(this), parent)
{
    D_THEME_INIT_WIDGET(dialogs/DDialog);

    d_func()->init();

    setTitle(title);
    setMessage(message);
}

int DDialog::getButtonIndexByText(const QString &text) const
{
    int i = -1;

    for(const QAbstractButton *button : getButtons()) {
        ++i;

        if(button->text() == text)
            return i;
    }

    return i;
}

int DDialog::buttonCount() const
{
    D_DC(DDialog);

    return d->buttonList.count();
}

int DDialog::contentCount() const
{
    D_DC(DDialog);

    return d->contentList.count();
}

QList<QAbstractButton*> DDialog::getButtons() const
{
    D_DC(DDialog);

    return d->buttonList;
}

QList<QWidget *> DDialog::getContents() const
{
    D_DC(DDialog);

    return d->contentList;
}

QAbstractButton *DDialog::getButton(int index) const
{
    D_DC(DDialog);

    return d->buttonList.at(index);
}

QWidget *DDialog::getContent(int index) const
{
    D_DC(DDialog);

    return d->contentList.at(index);
}

QString DDialog::title() const
{
    D_DC(DDialog);

    return d->title;
}

QString DDialog::message() const
{
    D_DC(DDialog);

    return d->message;
}

QIcon DDialog::icon() const
{
    D_DC(DDialog);

    return d->icon;
}

QPixmap DDialog::iconPixmap() const
{
    D_DC(DDialog);

    return *d->iconLabel->pixmap();
}

Qt::TextFormat DDialog::textFormat() const
{
    D_DC(DDialog);

    return d->textFormat;
}

bool DDialog::onButtonClickedClose() const
{
    D_DC(DDialog);

    return d->onButtonClickedClose;
}

int DDialog::addButton(const QString &text)
{
    int index = buttonCount();

    insertButton(index, text);

    return index;
}

int DDialog::addButtons(const QStringList &text)
{
    int index = buttonCount();

    insertButtons(index, text);

    return index;
}

void DDialog::insertButton(int index, const QString &text)
{
    QPushButton *button = new QPushButton(text);

    button->setObjectName("ActionButton");
    button->setAttribute(Qt::WA_NoMousePropagation);
    button->setFixedHeight(DIALOG::BUTTON_HEIGHT);

    insertButton(index, button);
}

void DDialog::insertButton(int index, QAbstractButton *button)
{
    D_D(DDialog);

    QLabel* label = new QLabel;
    label->setObjectName("VLine");
    label->setFixedWidth(1);
    label->hide();

    if(index > 0 && index >= buttonCount()) {
        QLabel *label = qobject_cast<QLabel*>(d->buttonLayout->itemAt(d->buttonLayout->count() - 1)->widget());
        if(label)
            label->show();
    }

    d->buttonLayout->insertWidget(index * 2, button);
    d->buttonList << button;
    d->buttonLayout->insertWidget(index * 2 + 1, label);

    connect(button, SIGNAL(clicked(bool)), this, SLOT(_q_onButtonClicked()));
}

void DDialog::insertButtons(int index, const QStringList &text)
{
    for(int i = 0; i < text.count(); ++i) {
        insertButton(index + i, text[i]);
    }
}

void DDialog::removeButton(int index)
{
    D_D(DDialog);

    QLabel *label = qobject_cast<QLabel*>(d->buttonLayout->itemAt(index * 2 + 1)->widget());
    QAbstractButton * button = qobject_cast<QAbstractButton*>(d->buttonLayout->itemAt(index * 2)->widget());

    if(label) {
        d->buttonLayout->removeWidget(label);
        label->deleteLater();
    }

    if(button) {
        d->buttonLayout->removeWidget(button);
        button->deleteLater();
    }

    if(index > 0 && index == buttonCount() - 1) {
        QLabel *label = qobject_cast<QLabel*>(d->buttonLayout->itemAt(d->buttonLayout->count() - 1)->widget());

        if(label)
            label->hide();
    }

    d->buttonList.removeAt(index);
}

void DDialog::removeButton(QAbstractButton *button)
{
    removeButton(getButtons().indexOf(button));
}

void DDialog::removeButtonByText(const QString &text)
{
    int index = getButtonIndexByText(text);

    if(index >= 0)
        removeButton(index);
}

void DDialog::clearButtons()
{
    D_D(DDialog);

    d->buttonList.clear();

    while(d->buttonLayout->count()) {
        QLayoutItem *item = d->buttonLayout->takeAt(0);

        item->widget()->deleteLater();
        delete item;
    }
}

void DDialog::addContent(QWidget *widget, Qt::Alignment alignment)
{
    D_DC(DDialog);

    int index = d->contentLayout->count();

    insertContent(index, widget, alignment);
}

void DDialog::insertContent(int index, QWidget *widget, Qt::Alignment alignment)
{
    D_D(DDialog);

    d->contentLayout->insertWidget(index + DIALOG::CONTENT_INSERT_OFFSET,
                                         widget, 0, alignment);
    d->contentList << widget;
}

void DDialog::removeContent(QWidget *widget, bool isDelete)
{
    D_D(DDialog);

    d->contentLayout->removeWidget(widget);

    if(isDelete)
        widget->deleteLater();

    d->contentList.removeOne(widget);
}

void DDialog::clearContents(bool isDelete)
{
    D_D(DDialog);

    for(QWidget *widget : d->contentList)
        d->contentLayout->removeWidget(widget);

    if(isDelete) {
        qDeleteAll(d->contentList);
    }

    d->contentList.clear();
}

void DDialog::setSpacing(int spacing)
{
    D_D(DDialog);

    d->contentLayout->setSpacing(spacing);
}

void DDialog::addSpacing(int spacing)
{
    D_D(DDialog);

    d->contentLayout->addSpacing(spacing);
}

void DDialog::insertSpacing(int index, int spacing)
{
    D_D(DDialog);

    d->contentLayout->insertSpacing(index, spacing);
}

void DDialog::clearSpacing()
{
    D_D(DDialog);

    for(int i = 0; i < d->contentLayout->count(); ++i) {
        QLayoutItem *item = d->contentLayout->itemAt(i);

        if(item->spacerItem()) {
            d->contentLayout->removeItem(item);
        }
    }
}

void DDialog::setButtonText(int index, const QString &text)
{
    QAbstractButton *button = getButton(index);

    button->setText(text);
}

void DDialog::setButtonIcon(int index, const QIcon &icon)
{
    QAbstractButton *button = getButton(index);

    button->setIcon(icon);
}

void DDialog::setTitle(const QString &title)
{
    D_D(DDialog);

    if (d->title == title)
        return;

    d->title = title;
    d->titleLabel->setText(title);
    d->titleLabel->setHidden(title.isEmpty());
    d->_q_updateLabelMaxWidth();

    emit titleChanged(title);
}

void DDialog::setMessage(const QString &message)
{
    D_D(DDialog);

    if (d->message == message)
        return;

    d->message = message;
    d->messageLabel->setText(message);
    d->messageLabel->setHidden(message.isEmpty());
    d->_q_updateLabelMaxWidth();

    emit messageChanged(message);
}

void DDialog::setIcon(const QIcon &icon)
{
    D_D(DDialog);

    d->icon = icon;

    if(!icon.isNull()) {
        const QList<QSize> &sizes = icon.availableSizes();

        if(!sizes.isEmpty())
            setIconPixmap(icon.pixmap(sizes.first()));
        else
            setIconPixmap(icon.pixmap(height() / 3));
    }
}

void DDialog::setIconPixmap(const QPixmap &iconPixmap)
{
    D_D(DDialog);

    d->iconLabel->setPixmap(iconPixmap);
    if(iconPixmap.isNull()) {
        d->iconLabel->hide();
        d->iconLayout->setSpacing(0);
    } else {
        d->iconLayout->setSpacing(DIALOG::ICON_LAYOUT_SPACING);
        d->iconLabel->show();
    }
}

void DDialog::setTextFormat(Qt::TextFormat textFormat)
{
    D_D(DDialog);

    if (d->textFormat == textFormat)
        return;

    d->textFormat = textFormat;
    d->titleLabel->setTextFormat(textFormat);
    d->messageLabel->setTextFormat(textFormat);

    emit textFormatChanged(textFormat);
}

void DDialog::setOnButtonClickedClose(bool onButtonClickedClose)
{
    D_D(DDialog);

    d->onButtonClickedClose = onButtonClickedClose;
}

int DDialog::exec()
{
    D_D(DDialog);

    d->clickedButtonIndex = -1;

    int code = DAbstractDialog::exec();

    return d->clickedButtonIndex >= 0 ? d->clickedButtonIndex : code;
}

DDialog::DDialog(DDialogPrivate &dd, QWidget *parent) :
    DAbstractDialog(dd, parent)
{
    D_THEME_INIT_WIDGET(dialogs/DDialog);

    d_func()->init();
}

void DDialog::closeEvent(QCloseEvent *event)
{
    emit aboutToClose();
    done(-1);
    DAbstractDialog::closeEvent(event);
    emit closed();
}

void DDialog::showEvent(QShowEvent *event)
{
    D_D(DDialog);

    DAbstractDialog::showEvent(event);

    d->_q_updateLabelMaxWidth();
}

DUI_END_NAMESPACE

#include "moc_ddialog.cpp"
