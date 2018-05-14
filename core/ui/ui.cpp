#include "ui.h"
#include "core.h"
#include "session/sessionrequest.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QStyle>
#include <QStyleFactory>
#include <interfaces/adamantplugin.h>

AdamantUI::AdamantUI(CoreService *parent)
    : QObject(parent)
    , _core(parent)
    , _theme(ApplicationTheme::Light) {
    setup();
    _window = new MainWindow(parent);

//    connect(_core->request(), &Session::Request::profileBadgeImage, _window, &MainWindow::onProfileBadgeImage);
//    connect(_core->request(), &Session::Request::profileData, [this](QString data) {
//        QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
//        if (doc.isObject() && !doc.isEmpty()) {
//            _window->updateAccountName(doc.object().value("name").toString());
//            _window->updateAccountMessagesCount(doc.object().value("messages").toInt());
//        }
//    });
}

void AdamantUI::setup() {
    qApp->setStyle(QStyleFactory::create("fusion"));

    _lightPalette = qApp->palette();
    _darkPalette = qApp->palette();
    _darkPalette.setColor(QPalette::Window,             QColor(53, 53, 53));
    _darkPalette.setColor(QPalette::WindowText,         Qt::white);
    _darkPalette.setColor(QPalette::Base,               QColor(15,15,15));
    _darkPalette.setColor(QPalette::AlternateBase,      QColor(53,53,53));
    _darkPalette.setColor(QPalette::ToolTipBase,        Qt::white);
    _darkPalette.setColor(QPalette::ToolTipText,        Qt::white);
    _darkPalette.setColor(QPalette::Text,               Qt::white);
    _darkPalette.setColor(QPalette::Button,             QColor(53,53,53));
    _darkPalette.setColor(QPalette::ButtonText,         Qt::white);
    _darkPalette.setColor(QPalette::BrightText,         Qt::red);
    _darkPalette.setColor(QPalette::Highlight,          QColor(142,45,197).lighter());
    _darkPalette.setColor(QPalette::HighlightedText,    Qt::black);
    _darkPalette.setColor(QPalette::Disabled,           QPalette::Text, Qt::darkGray);
    _darkPalette.setColor(QPalette::Disabled,           QPalette::ButtonText, Qt::darkGray);

    //qApp->setPalette(_lightPalette);
}

AdamantUI::~AdamantUI() {
    _window->deleteLater();
}

QUuid AdamantUI::registerPluginPage(AdamantPlugin* plugin, const QIcon& icon, const QString& title, const QString& description, QWidget* widget) {
    QUuid id = window()->registerPage(icon, title, description, widget, plugin);
    connect(plugin, &AdamantPlugin::destroyed, this, [this, id]() {
        // Plugin is about to be destroyed, clean up quick!
        window()->removePage(id);
    });

    return id;
}

void AdamantUI::setTheme(ApplicationTheme theme) {
    if (theme == _theme) return;
    ApplicationTheme old = _theme;
    _theme = theme;

    QPalette p;
    if (theme == ApplicationTheme::Dark)        p = _darkPalette;
    else if (theme == ApplicationTheme::Light)  p = _lightPalette;

    window()->setPalette(p);
    {
        QList<QWidget*> widgets = window()->findChildren<QWidget*>();
        foreach (QWidget* w, widgets)
            w->setPalette(p);
    }
    qApp->setPalette(p);

    emit applicationThemeChanged(old, _theme);
}

void AdamantUI::start() {
    _window->show();
}
