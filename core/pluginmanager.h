#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <core_global.h>
#include "adamantplugininfo.h"

#include <QDir>
#include <QObject>
#include <QPluginLoader>
#include <QList>
#include <QMetaObject>
#include <QThread>
#include <QTimer>

class CoreService;
class AdamantPlugin;
class ScriptSandbox;
class QNetworkAccessManager;

typedef const QList<const AdamantPlugin*> PluginList;

class CORE_EXTERN PluginManager : public QObject
{
    Q_OBJECT
public:
    explicit PluginManager(CoreService *parent = 0);

    const QDir pluginsPath() const;

    void scanPlugins(bool initialScan = true);
    void verifyPlugins();
    void preparePlugins();
    bool injectPluginData(AdamantPluginInfo *plugin);
    void loadPlugins();

    AdamantPlugin* getPluginByIID(const QString &iid) const {
        for (const AdamantPluginInfo* data : _plugins) {
            QString pIid = data->metaData.value("IID").toString();
            if (pIid.compare(iid, Qt::CaseInsensitive) == 0 &&
                data->instance != 0) {
                return data->instance;
            }
        }
        return nullptr;
    }

    QList<AdamantPluginInfo*> getPluginContainers() const {
        return _plugins;
    }

    PluginList getPlugins() const {
        QList<const AdamantPlugin*> plugins;
        for (const AdamantPluginInfo* data : _plugins) {
            if (data->instance != 0)
                plugins.append(data->instance);
        }
        return plugins;
    }

    CoreService* core() const {
        return _parent;
    }

    const QJsonObject getPluginMetaData(const AdamantPlugin *plugin) const;
    AdamantPluginInfo* getPluginData(const AdamantPlugin *plugin) const;

    void finish();
    void verifyPlugin(AdamantPluginInfo *plugin);

    void loadAndExecuteScripts();
    void loadScripts();
signals:
    void pluginDiscovered(QString);

    void beginningPluginVerification();
    void pluginStateChanged(QString, PluginState);
    void pluginOutdated(QString, QString, QString);
    void finishingPluginVerification();

    void pluginLoadStarted(QString);
    void pluginLoadingFinished();

    void pluginMessage(QString, QtMsgType) const;
public slots:
    ScriptSandbox* addScript(const QString &file, const QString &script, AdamantPlugin *owner = 0);
    void reloadScripts();
    void onPluginMessage(const AdamantPlugin* plugin, QString message, QtMsgType type) const;
private slots:
    void onScriptFinished(ScriptSandbox *s);
    void onScriptOutput(const QString &output);
private:
    CoreService* _parent;
    QDir _pluginPath;
    QDir _scriptPath;
    QNetworkAccessManager* _networkManager;

    QList<AdamantPluginInfo*> _plugins;
    QMultiHash<QString, AdamantPluginInfo*> _pluginsByRole;

    QList<ScriptSandbox*> _scripts;
};

Q_DECLARE_OPAQUE_POINTER(PluginManager*)
Q_DECLARE_METATYPE(PluginManager*)

#endif // PLUGINMANAGER_H
