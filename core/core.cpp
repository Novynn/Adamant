#include "core.h"
#include "pluginmanager.h"
#include "session/psession.h"
#include "ui/ui.h"
#include <items/itemmanager.h>
#include <ui/pages/pluginpage.h>

CoreService::CoreService()
    : QObject()
    , _pluginManager(new PluginManager(this))
    , _settings("plugins.ini", QSettings::IniFormat)
    , _sensitiveSettings("sensitive.ini", QSettings::IniFormat)
    , _session(new PSession(this,
                            _sensitiveSettings.value("account").toString(),
                            _sensitiveSettings.value("id").toString()))
    , _itemManager(new ItemManager(this)) {

    _ui = new UI(this);

    connect(_pluginManager, &PluginManager::PluginMessage, [this] (QString message, QtMsgType type) {
        emit Message(message, type);
    });
}

CoreService::~CoreService() {
}

void CoreService::Load() {
    SensitiveSettings()->beginGroup("session");
    QString sessionId = SensitiveSettings()->value("id").toString();
    SensitiveSettings()->endGroup();

    // Required here so that the setup dialog also gets this palette
    Interface()->SetPalette();

    if (sessionId.isEmpty()) {
        // Oh no, run setup.
        int result = Interface()->ShowSetup(); // Blocking
        if (result != 0x0) {
            qApp->quit();
            return;
        }

        QVariantMap map = Interface()->GetSetupDialog()->GetData();

        // Setup completed, we should have valid data
        SensitiveSettings()->beginGroup("session");
        for (QString key : map.uniqueKeys()) {
            SensitiveSettings()->setValue(key, map.value(key));
        }
        sessionId = SensitiveSettings()->value("id").toString();
        SensitiveSettings()->endGroup();
    }

    // Load the main application!
    connect(_session, &PSession::ProfileData, this, ProfileLoaded);
    Session()->LoginWithSessionId(sessionId);
    Interface()->OnLoad();
}

void CoreService::ProfileLoaded(QString profileData) {
    Q_UNUSED(profileData)
    // We only care about this once...
    disconnect(_session, &PSession::ProfileData, this, &CoreService::ProfileLoaded);

    // Load Plugins
    GetPluginManager()->ScanPlugins(true);
    GetPluginManager()->VerifyPlugins();
    GetPluginManager()->PreparePlugins();
    GetPluginManager()->LoadPlugins();

    emit Message("Adamant Started!", QtInfoMsg);

    Interface()->RegisterPages();
}

void CoreService::LoggedMessage(const QString &message, QtMsgType type) {
    QString sType;
    switch (type) {
        case QtDebugMsg:      sType = "DEB"; break;
        case QtInfoMsg:       sType = "LOG"; break;
        case QtWarningMsg:    sType = "WAR"; break;
        case QtCriticalMsg:   sType = "ERR"; break;
        case QtFatalMsg:      sType = "FAT"; break;
    }

    Interface()->Window()->AppendScriptOutput(message, sType);
}

QDir CoreService::applicationPath() {
    return QDir(QApplication::applicationDirPath());
}
