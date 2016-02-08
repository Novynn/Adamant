#include "ui.h"
#include "core.h"
#include "session/sessionrequest.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QStyle>
#include <QStyleFactory>

UI::UI(CoreService *parent)
    : QObject(parent)
    , _core(parent)
    , _theme(ApplicationTheme::Light) {
    _window = new MainWindow(parent);
    _setupDialog = new SetupDialog(_window);

    connect(_setupDialog, &SetupDialog::loginByIdRequested,
            _core->session(), &Session::Request::loginWithSessionId);
    connect(_setupDialog, &SetupDialog::loginRequested,
            _core->session(), &Session::Request::login);
    connect(this, &UI::requestProfileData,
            _core->session(), &Session::Request::loginWithSessionId);

    connect(_core->session(), &Session::Request::loginResult,
            [this] (int result, QString resultString) {
        if (result == 0) {
            QString sessionId = _core->session()->sessionId();
            _setupDialog->loginSuccess(sessionId);
        }
        else {
            _setupDialog->loginFailed(resultString);
        }
    });

    connect(_core->session(), &Session::Request::profileBadgeImage, _window, &MainWindow::onProfileBadgeImage);
    connect(_core->session(), &Session::Request::profileAvatarImage, _setupDialog, &SetupDialog::updateAccountAvatar);
    connect(_core->session(), &Session::Request::profileData, [this](QString data) {
        QJsonDocument doc = QJsonDocument::fromJson(data.toLatin1());
        if (doc.isObject() && !doc.isEmpty()) {
            _setupDialog->updateAccountName(doc.object().value("name").toString());
            _window->updateAccountMessagesCount(doc.object().value("messages").toInt());
        }
    });

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

    qApp->setPalette(_lightPalette);
}

UI::~UI() {
    _window->deleteLater();
    _setupDialog->deleteLater();
}

void UI::setTheme(ApplicationTheme theme) {
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

    getSetupDialog()->setPalette(p);
    {
        QList<QWidget*> widgets = getSetupDialog()->findChildren<QWidget*>();
        foreach (QWidget* w, widgets)
            w->setPalette(p);
    }
    qApp->setPalette(p);

    emit applicationThemeChanged(old, _theme);
}

void UI::start() {
    _window->show();
}

int UI::showSetup() {
    return _setupDialog->exec();
}
