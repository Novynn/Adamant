#ifndef SETTINGS_H
#define SETTINGS_H

#include <core_global.h>
#include <QObject>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>
#include <QSettings>

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
        return QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    }

    static QDir dataPath(const QString &folder) {
        QDir path = CoreService::dataPath();
        if (!path.cd(folder)) {
            path.mkdir(folder);
            path.cd(folder);
        }
        return path;
    }

    static QDir cachePath() {
        return dataPath("cache");
    }

    bool load();

    Q_INVOKABLE AdamantUI* getInterface() {
        return _ui;
    }

    Q_INVOKABLE ScriptSandbox* script() {
        return _script;
    }

    Q_INVOKABLE Session::Request* session() {
        return Session::Global();
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
signals:
    void message(const QString &message, QtMsgType type);
private slots:
    void ready();
public slots:
    void loggedMessage(const QString &message, QtMsgType type);
private:
    PluginManager* _pluginManager;
    QSettings _settings;
    QSettings _sensitiveSettings;
    ItemManager* _itemManager;
    ScriptSandbox* _script;
    AdamantUI* _ui;

    QMap<QString, QMetaObject::Connection> _requiredData;
};

Q_DECLARE_METATYPE(CoreService*)

#endif // SETTINGS_H
