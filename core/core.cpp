#include "core.h"
#include "pluginmanager.h"
#include "ui/ui.h"
#include <items/itemmanager.h>
#include <ui/pages/pluginpage.h>
#include "session/sessionrequest.h"

CoreService::CoreService()
    : QObject()
    , _pluginManager(new PluginManager(this))
    , _settings("plugins.ini", QSettings::IniFormat)
    , _sensitiveSettings("sensitive.ini", QSettings::IniFormat)
    , _itemManager(new ItemManager(this)) {
    Session::SetCoreService(this);

    _ui = new UI(this);

    connect(_pluginManager, &PluginManager::pluginMessage, [this] (QString msg, QtMsgType type) {
        emit message(msg, type);
    });
}

CoreService::~CoreService() {
}

void CoreService::load() {
    sensitiveSettings()->beginGroup("session");
    QString sessionId = sensitiveSettings()->value("id").toString();
    sensitiveSettings()->endGroup();

    // Required here so that the setup dialog also gets this palette
    interface()->setTheme();

    while (sessionId.isEmpty()) {
        // Oh no, run setup.
        int result = interface()->showSetup(); // Blocking
        if (result != 0x0) {
            qApp->quit();
            return;
        }

        QVariantMap map = interface()->getSetupDialog()->getData();

        // Setup completed, we should have valid data
        sensitiveSettings()->beginGroup("session");
        for (QString key : map.uniqueKeys()) {
            sensitiveSettings()->setValue(key, map.value(key));
        }
        sessionId = sensitiveSettings()->value("id").toString();
        sensitiveSettings()->endGroup();
    }

    // Load the main application!
    connect(session(), &Session::Request::profileData, this, profileLoaded);
    session()->loginWithSessionId(sessionId);
    interface()->start();

    // Load Plugins
    getPluginManager()->scanPlugins(true);
    getPluginManager()->verifyPlugins();
    getPluginManager()->preparePlugins();
}

void CoreService::profileLoaded(QString profileData) {
    Q_UNUSED(profileData)
    // We only care about this once...
    disconnect(session(), &Session::Request::profileData, this, &CoreService::profileLoaded);

    getPluginManager()->loadPlugins();

    emit message("Adamant Started!", QtInfoMsg);
    interface()->registerPages();

    // Set page to home
    interface()->window()->setPageIndex(0);
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

    interface()->window()->appendScriptOutput(message, sType);
}

QDir CoreService::applicationPath() {
    return QDir(QApplication::applicationDirPath());
}
