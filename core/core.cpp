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
    , _settings("data.ini", QSettings::IniFormat)
    , _sensitiveSettings("sensitive.ini", QSettings::IniFormat)
    , _session(new Session(this))
    , _itemManager(new ItemManager(this))
    , _script(new ScriptSandbox(_pluginManager, "console", "")) {

    _ui = new AdamantUI(this);

    connect(_pluginManager, &PluginManager::pluginMessage, [this] (QString msg, QtMsgType type) {
        emit message(msg, type);
    });

    _script->setup();
}

CoreService::~CoreService() {
}

SetupDialog::LoginMethod CoreService::setup(bool force) {
    sensitiveSettings()->beginGroup("session");
    QString sessionId = sensitiveSettings()->value("id").toString();
    QString accessToken = sensitiveSettings()->value("access_token").toString();
    sensitiveSettings()->endGroup();

    SetupDialog::LoginMethod method = SetupDialog::LoginSessionId;
    if (!accessToken.isEmpty()) {
        method = SetupDialog::LoginOAuth;
    }

    while ((sessionId.isEmpty() && accessToken.isEmpty()) || force) {
        getInterface()->window()->hide();
        int result = getInterface()->showSetup(); // Blocking
        if (result != 0x0) {
            getInterface()->window()->close();
            return SetupDialog::LoginNone;
        }
        getInterface()->window()->show();
        force = false;

        QVariantMap map = getInterface()->getSetupDialog()->getData();

        // Setup completed, we should have valid data
        sensitiveSettings()->beginGroup("session");
        for (QString key : map.uniqueKeys()) {
            sensitiveSettings()->setValue(key, map.value(key));
        }
        sessionId = map.value("id").toString();
        sensitiveSettings()->endGroup();

        method = (SetupDialog::LoginMethod)map.value("method").toInt();
        if (method == SetupDialog::LoginOAuth && !map.value("access_token").toString().isEmpty()) {
            break;
        }
    }

    return method;
}

bool CoreService::login(SetupDialog::LoginMethod method) {
    connect(session(), &Session::sessionChange, this, &CoreService::sessionChange);

    sensitiveSettings()->beginGroup("session");
    QString sessionId = sensitiveSettings()->value("id").toString();
    QString accessToken = sensitiveSettings()->value("access_token").toString();
    sensitiveSettings()->endGroup();

    // Attempt to log in
    switch (method) {
        case SetupDialog::LoginSessionId: {
            request()->loginWithSessionId(sessionId);
        } break;
        case SetupDialog::LoginOAuth: {
            Q_UNUSED(accessToken);
            qFatal("OAuth not implemented!");
        } break;
        case SetupDialog::LoginNone:
        default: {
            qFatal("Logging in without a method?");
        }
    }

    _ui->window()->setLoginProgressMessage("Logging in...");
    return true;
}

bool CoreService::load(bool force) {
    // Required here so that the setup dialog also gets this palette
    getInterface()->setTheme();
    getInterface()->start();

    request()->fetchLeagues();

    auto method = setup(force);

    if (method == SetupDialog::LoginNone)
        return false;

    // Load Plugins
    getPluginManager()->scanPlugins(true);
    getPluginManager()->verifyPlugins();
    getPluginManager()->preparePlugins();

    return login(method);
}

void CoreService::sessionChange() {
    if (session()->loginState() == Session::Success) {
        bool finished = true;

        const QString sessionId = session()->sessionId();
        if (!sessionId.isEmpty()) {
            sensitiveSettings()->beginGroup("session");
            sensitiveSettings()->setValue("id", sessionId);
            sensitiveSettings()->endGroup();
        }
        else {
            finished = false;
        }

        const QString name = session()->accountName();
        if (!name.isEmpty()) {
            sensitiveSettings()->beginGroup("session");
            sensitiveSettings()->setValue("name", name);
            sensitiveSettings()->endGroup();
        }
        else {
            finished = false;
        }

        const QStringList leagues = session()->leagues();
        if (leagues.isEmpty()) {
            finished = false;
        }

        sensitiveSettings()->sync();

        if (!finished) {
            return;
        }
        disconnect(session(), &Session::sessionChange, this, &CoreService::sessionChange);

        ready();
    }
    else if (session()->loginState() == Session::Failed){
        _ui->window()->setLoginProgressMessage("Failed to log in.");
        disconnect(session(), &Session::sessionChange, this, &CoreService::sessionChange);
        session()->resetLoginState();

        // NOTE(rory) Infinite loop?
        auto method = setup(true);
        if (method == SetupDialog::LoginNone)
            return;
        login(method);
    }
}

void CoreService::ready() {
    getPluginManager()->loadPlugins();

    qInfo() << qPrintable("Adamant Started!");
    getInterface()->registerPages();

    // Set page to home
    getInterface()->window()->setPageIndex(0);
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
