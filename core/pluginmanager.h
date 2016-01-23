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

    const QDir PluginsPath() const;

    void ScanPlugins(bool initialScan = true);
    void VerifyPlugins();
    void PreparePlugins();
    bool InjectPluginData(AdamantPluginInfo *plugin);
    void LoadPlugins();

    AdamantPlugin* GetPluginByIID(const QString &iid) const {
        for (const AdamantPluginInfo* data : _plugins) {
            QString pIid = data->metaData.value("IID").toString();
            if (pIid.compare(iid, Qt::CaseInsensitive) == 0 &&
                data->instance != 0) {
                return data->instance;
            }
        }
        return 0;
    }

    QList<AdamantPluginInfo*> GetPluginContainers() const {
        return _plugins;
    }

    PluginList GetPlugins() const {
        QList<const AdamantPlugin*> plugins;
        for (const AdamantPluginInfo* data : _plugins) {
            if (data->instance != 0)
                plugins.append(data->instance);
        }
        return plugins;
    }

    CoreService* Core() const {
        return _parent;
    }

    const QJsonObject GetPluginMetaData(const AdamantPlugin *plugin) const;
    AdamantPluginInfo* GetPluginData(const AdamantPlugin *plugin) const;

    void Finish();
    void VerifyPlugin(AdamantPluginInfo *plugin);

    void LoadAndExecuteScripts();
    void LoadScripts();
signals:
    void PluginDiscovered(QString);

    void BeginningPluginVerification();
    void PluginStateChanged(QString, PluginState);
    void PluginOutdated(QString, QString, QString);
    void FinishingPluginVerification();

    void PluginLoadStarted(QString);
    void PluginLoadingFinished();

    void PluginMessage(QString, QtMsgType) const;
public slots:
    ScriptSandbox* AddScript(const QString &script, AdamantPlugin *owner = 0);
    void ReloadScripts();
    void OnPluginMessage(const AdamantPlugin* plugin, QString message, QtMsgType type) const;
private slots:
    void onScriptFinished(ScriptSandbox *s);
    void OnScriptOutput(const QString &output);
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
