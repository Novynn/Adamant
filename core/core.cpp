#include "core.h"
#include "pluginmanager.h"
#include <ui/ui.h>
#include <items/itemmanager.h>
#include <ui/pages/pluginpage.h>
#include <session/sessionrequest.h>
#include <scripting/scriptsandbox.h>

CoreService::CoreService()
    : QObject()
    , _pluginManager(new PluginManager(this))
    , _oauthHandler()
    , _settings("data.ini", QSettings::IniFormat)
    , _session(new Session(this))
    , _itemManager(new ItemManager(this))
    , _script(new ScriptSandbox(_pluginManager, "console", ""))
    , _ready(false) {

    _ui = new AdamantUI(this);

    connect(_pluginManager, &PluginManager::pluginMessage, this, [this] (QString msg, QtMsgType type) {
        emit message(msg, type);
    });

    connect(_session, &Session::loginStateChange, this, &CoreService::loginStateChange);

    connect(&_oauthHandler, &OAuthHandler::oauthCodeAcquired, this, [this] (const QString &authCode) {
        _ui->window()->setLoginProgressMessage("Authenticating...");
        Session::LoginWithOAuth(session(), authCode);
    });

    _script->setup();
}

CoreService::~CoreService() {
}

bool CoreService::load(bool force) {
    // Required here so that the setup dialog also gets this palette
    getInterface()->setTheme();
    getInterface()->start();

    Session::Request::FetchLeagues(session(), [this](Session* session, const QStringList &leagues) {
        session->updateLeagues(leagues);

        ready();
    });

    if (_oauthHandler.hasAccessToken()) {
        login(_oauthHandler.getAccessToken());
    }

    return true;
}

void CoreService::reset() {
    _oauthHandler.save(QString());
    session()->resetLoginState();
    _ui->window()->switchToSignIn();
}

bool CoreService::login(const QString &accessToken) {
    _ui->window()->setLoginProgressMessage("Logging in...");
    _ui->window()->switchToLoadingFromSignIn();

    Session::LoginWithOAuthAccessToken(session(), accessToken);
    return true;
}

void CoreService::newLogin() {
    if (_oauthHandler.listen()) {
        _ui->window()->setLoginProgressMessage("Waiting for authorization...");
    }
    _oauthHandler.signIn();
}

void CoreService::loginStateChange(Session::SessionLoginState state) {
    qDebug() << "loginStateChange" << state;
    switch (state) {
        case Session::Idle: {
        } break;
        case Session::Failed: {
            _ui->window()->setLoginProgressMessage("Failed to log in.");
            QTimer::singleShot(1000, this, &CoreService::reset);
        } break;
        case Session::Success: {
            _ui->window()->setLoginProgressMessage("Logged in successfully.");
            _oauthHandler.save(session()->accessToken());

            ready();
        } break;
    }
}

void CoreService::ready() {
    if (session()->leagues().isEmpty()) {
        qWarning() << qPrintable("No leagues yet...");
        return;
    }

    if (!_ready) {
        _ready = true;

        // Load Plugins
        qInfo() << qPrintable("Checking plugins...");
        getPluginManager()->scanPlugins(true);
        getPluginManager()->verifyPlugins();
        getPluginManager()->preparePlugins();

        qInfo() << qPrintable("Loading plugins...");
        getPluginManager()->loadPlugins();

        qInfo() << qPrintable("Adamant Started!");
        getInterface()->registerPages();

        // Set page to home
        getInterface()->window()->setPageIndex(0);
    }
}

void CoreService::loggedMessage(const QString &message, QtMsgType type) {
    QString sType;
    switch (type) {
        case QtDebugMsg:      sType = "DEB"; break;
        case QtInfoMsg:       sType = "LOG"; break;
        case QtWarningMsg:    sType = "WAR"; break;
        case QtCriticalMsg:   sType = "ERR"; break;
        case QtFatalMsg:      sType = "FAT"; break;
    }

    getInterface()->window()->appendScriptOutput(message, sType);
}

QDir CoreService::applicationPath() {
    return QDir(QApplication::applicationDirPath());
}
