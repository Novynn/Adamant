#include "ui.h"
#include "core.h"
#include "session/psession.h"
#include <QStyle>
#include <QStyleFactory>

UI::UI(CoreService *parent)
    : QObject(parent)
    , _core(parent)
    , _theme(ApplicationTheme::Light) {
    _setupDialog = new SetupDialog();
    _window = new MainWindow(parent);

    connect(_setupDialog, &SetupDialog::LoginByIdRequested,
            _core->Session(), &PSession::LoginWithSessionId);
    connect(_setupDialog, &SetupDialog::LoginRequested,
            _core->Session(), &PSession::Login);
    connect(this, &UI::RequestProfileData,
            _core->Session(), &PSession::LoginWithSessionId);

    connect(_core->Session(), &PSession::LoginResult,
            [this] (int result, QString resultString) {
        if (result == 0) {
            QString sessionId = _core->Session()->property("SessionId").toString();
            _setupDialog->LoginSuccess(sessionId);
        }
        else {
            _setupDialog->LoginFailed(resultString);
        }
    });

    connect(_core->Session(), &PSession::ProfileBadgeImage, _window, &MainWindow::OnProfileBadgeImage);
    connect(_core->Session(), &PSession::ProfileAvatarImage, _setupDialog, &SetupDialog::UpdateAccountAvatar);
    connect(_core->Session(), &PSession::ProfileData, [this](QString data) {
        QJsonDocument doc = QJsonDocument::fromJson(data.toLatin1());
        if (doc.isObject() && !doc.isEmpty()) {
            _setupDialog->UpdateAccountName(doc.object().value("name").toString());
            _window->UpdateAccountMessagesCount(doc.object().value("messages").toInt());
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

void UI::SetPalette(ApplicationTheme theme) {
    if (theme == _theme) return;
    ApplicationTheme old = _theme;
    _theme = theme;

    QPalette p;
    if (theme == ApplicationTheme::Dark)        p = _darkPalette;
    else if (theme == ApplicationTheme::Light)  p = _lightPalette;

    Window()->setPalette(p);
    {
        QList<QWidget*> widgets = Window()->findChildren<QWidget*>();
        foreach (QWidget* w, widgets)
            w->setPalette(p);
    }

    GetSetupDialog()->setPalette(p);
    {
        QList<QWidget*> widgets = GetSetupDialog()->findChildren<QWidget*>();
        foreach (QWidget* w, widgets)
            w->setPalette(p);
    }
    qApp->setPalette(p);

    emit ApplicationThemeChanged(old, _theme);
}

void UI::OnLoad() {
    _window->show();
}

int UI::ShowSetup() {
    return _setupDialog->exec();
}
