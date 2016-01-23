#include "ui.h"
#include "core.h"
#include "session/psession.h"
#include <QStyleFactory>

UI::UI(CoreService *parent)
    : QObject(parent)
    , _core(parent) {
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
}

void UI::SetPalette() {
    qApp->setStyle(QStyleFactory::create("fusion"));

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(53, 53, 53));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor(15,15,15));
    palette.setColor(QPalette::AlternateBase, QColor(53,53,53));
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, QColor(53,53,53));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);

    palette.setColor(QPalette::Highlight, QColor(142,45,197).lighter());
    palette.setColor(QPalette::HighlightedText, Qt::black);

    palette.setColor(QPalette::Disabled, QPalette::Text, Qt::darkGray);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::darkGray);
    qApp->setPalette(palette);
}

void UI::OnLoad() {
    SetPalette();

    _window->show();
}

int UI::ShowSetup() {
    return _setupDialog->exec();
}
