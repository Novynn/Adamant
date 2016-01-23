#ifndef SETTINGS_H
#define SETTINGS_H

#include <core_global.h>
#include <QObject>
#include <QSettings>
#include <QDir>
#include <QStandardPaths>
#include <QSettings>

class PSession;
class UI;
class Adamant;
class PluginManager;
class ItemManager;

class CORE_EXTERN CoreService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(PSession* session MEMBER _session)
    Q_PROPERTY(UI* ui MEMBER _ui)
    Q_PROPERTY(ItemManager* item_manager MEMBER _itemManager)
public:
    CoreService();

    static QDir applicationPath();

    static QDir dataPath() {
        return QDir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    }

    void Load();

    UI* Interface() {
        return _ui;
    }

    PSession* Session() {
        return _session;
    }

    Q_INVOKABLE QSettings* Settings() {
        return &_settings;
    }

    Q_INVOKABLE QSettings* SensitiveSettings() {
        return &_sensitiveSettings;
    }

    Q_INVOKABLE PluginManager* GetPluginManager() {
        return _pluginManager;
    }

    Q_INVOKABLE ItemManager* GetItemManager() {
        return _itemManager;
    }
signals:
    void Message(const QString &message, QtMsgType type);
private slots:
    void ProfileLoaded(QString profileData);
public slots:
    void LoggedMessage(const QString &message, QtMsgType type);
private:
    PluginManager* _pluginManager;
    QSettings _settings;
    QSettings _sensitiveSettings;
    PSession* _session;
    ItemManager* _itemManager;
    UI* _ui;
};

Q_DECLARE_METATYPE(CoreService*)

#endif // SETTINGS_H
