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
    , _itemManager(new ItemManager(this))
    , _script(new ScriptSandbox(_pluginManager, "console", "")) {
    Session::SetCoreService(this);

    _ui = new AdamantUI(this);

    connect(_pluginManager, &PluginManager::pluginMessage, [this] (QString msg, QtMsgType type) {
        emit message(msg, type);
    });

    _script->setup();
}

CoreService::~CoreService() {
}

#define ADD_REQUIREMENT(object, slot, type, var) \
    _requiredData.insert(#slot, QObject::connect(object, &slot, this, [this] (type var) { \
        QObject::disconnect(_requiredData.take(#slot)); \
        _settings.beginGroup("data"); \
        _settings.setValue(#var, var); \
        _settings.endGroup(); \
        if (_requiredData.isEmpty()) { \
            ready(); \
        } \
    }))

bool CoreService::load() {
    sensitiveSettings()->beginGroup("session");
    QString sessionId = sensitiveSettings()->value("id").toString();
    sensitiveSettings()->endGroup();

    // Required here so that the setup dialog also gets this palette
    getInterface()->setTheme();

    getInterface()->start();

    while (sessionId.isEmpty()) {
        // Oh no, run setup.
        int result = getInterface()->showSetup(); // Blocking
        if (result != 0x0) {
            getInterface()->window()->close();
            return false;
        }

        QVariantMap map = getInterface()->getSetupDialog()->getData();

        // Setup completed, we should have valid data
        sensitiveSettings()->beginGroup("session");
        for (QString key : map.uniqueKeys()) {
            sensitiveSettings()->setValue(key, map.value(key));
        }
        sessionId = sensitiveSettings()->value("id").toString();
        sensitiveSettings()->endGroup();
    }

    // These are requirements for data we need before the application loads
    _requiredData.clear();

    // Load Plugins
    getPluginManager()->scanPlugins(true);
    getPluginManager()->verifyPlugins();
    getPluginManager()->preparePlugins();

    // Load the main application!
    /* Require profile data */ {
        ADD_REQUIREMENT(session(), Session::Request::profileData, QString, profile);
        session()->loginWithSessionId(sessionId);

        // TODO(rory): Improve this
        connect(session(), &Session::Request::loginResult,
                [this] (int result, QString resultString) {
            if (result == 0) {

            }
            else {
                qDebug() << "Failed to log in: " << resultString;
            }
        });
    }

    /* Require leagues list */ {
        ADD_REQUIREMENT(session(), Session::Request::leaguesList, QStringList, leagues);
        session()->fetchLeagues();
    }

    return true;
}

void CoreService::ready() {
    _settings.beginGroup("data");
    QString profileData = _settings.value("profile").toString();
    QStringList leagues = _settings.value("leagues").toStringList();
    _settings.endGroup();

    qDebug() << leagues;

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
