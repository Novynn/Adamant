#include "custompage.h"
#include <ui/commandbutton.h>
#include <QShortcut>

CustomPage::CustomPage(QUuid id, const QIcon &icon, const QString &title, const QString &description,
                       QWidget *widget, QObject* owner)
    : _id(id)
    , _widget(widget)
    , _owner(owner)
{
    _button = new CommandButton(widget);
    _button->setText(title);
    _button->setDescription(description);
    _button->setToolTip(description);
    _button->setIcon(icon);
    _button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    _button->setMinimumSize(30, 36);
    _button->setMaximumSize(16777215, 36);
    _button->setCheckable(true);

    connect(_button, &CommandButton::clicked, this, &CustomPage::activate);
}

void CustomPage::activate() {
    emit activated(_id);
}
