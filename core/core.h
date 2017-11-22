#ifndef SETTINGS_H
#define SETTINGS_H

#include <core_global.h>
#include <QObject>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include <QDebug>

#include "ui/setupdialog.h"
#include "session/session.h"
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

    Q_INVOKABLE Session::Request* request() {
        return _session->request();
    }

    Q_INVOKABLE Session::ForumRequest* forum() {
        return _session->forum();
    }

    Q_INVOKABLE QSettings* settings() {
        return &_settings;
    }

    Q_INVOKABLE QSettings* sensitiveSettings() {
        return &_sensitiveSettings;
    }

    Q_INVOKABLE PluginManager* getPluginManager() {
        return _pluginManager;
    }

    Q_INVOKABLE ItemManager* getItemManager() {
        return _itemManager;
    }
protected:
    bool login(SetupDialog::LoginMethod method);
    SetupDialog::LoginMethod setup(bool force);
signals:
    void message(const QString &message, QtMsgType type);
private slots:
    void ready();
public slots:
    void loggedMessage(const QString &message, QtMsgType type);
    void sessionChange();
private:
    PluginManager* _pluginManager;
    QSettings _settings;
    QSettings _sensitiveSettings;
    Session* _session;
    ItemManager* _itemManager;
    ScriptSandbox* _script;
    AdamantUI* _ui;
};

Q_DECLARE_METATYPE(CoreService*)

#endif // SETTINGS_H
