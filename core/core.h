#ifndef SETTINGS_H
#define SETTINGS_H

#include <core_global.h>
#include <QObject>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include <QDebug>

#include "session/session.h"
#include "oauthhandler.h"
class AdamantUI;
class Adamant;
class PluginManager;
class ItemManager;
class ScriptSandbox;

class CORE_EXTERN CoreService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(AdamantUI* ui MEMBER _ui)
    Q_PROPERTY(ItemManager* item_manager MEMBER _itemManager)
public:
    CoreService();
    ~CoreService();

    static QDir applicationPath();

    static QDir dataPath() {
        auto dir = QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));

        if (!dir.exists()) dir.mkpath(dir.absolutePath());

        return dir;
    }

    static QDir dataPath(const QString &folder) {
        QDir path = CoreService::dataPath();
        if (!path.cd(folder)) {
            if (!path.mkdir(folder)) {
                auto info = QFileInfo(path.absolutePath());
                qWarning() << "Could not make folder" << folder;
            }
            path.cd(folder);
        }
        return path;
    }

    static QDir cachePath() {
        return dataPath("cache");
    }

    bool load(bool force = false);

    Q_INVOKABLE AdamantUI* getInterface() {
        return _ui;
    }

    Q_INVOKABLE ScriptSandbox* script() {
        return _script;
    }

    Q_INVOKABLE Session* session() {
        return _session;
    }

    Q_INVOKABLE QSettings* settings() {
        return &_settings;
    }

    Q_INVOKABLE PluginManager* getPluginManager() {
        return _pluginManager;
    }

    Q_INVOKABLE ItemManager* getItemManager() {
        return _itemManager;
    }
    void newLogin();
protected:
    bool login(const QString &accessToken);
    bool setup(bool force);
signals:
    void message(const QString &message, QtMsgType type);
private slots:
    void ready();
public slots:
    void loggedMessage(const QString &message, QtMsgType type);
    void loginStateChange(Session::SessionLoginState state);
    void reset();
private:
    PluginManager* _pluginManager;
    OAuthHandler _oauthHandler;
    QSettings _settings;
    Session* _session;
    ItemManager* _itemManager;
    ScriptSandbox* _script;
    AdamantUI* _ui;

    bool _ready;
};

Q_DECLARE_METATYPE(CoreService*)

#endif // SETTINGS_H
