#include "pluginmanager.h"
#include <QApplication>
#include <QMetaObject>
#include <QMetaProperty>
#include <QQueue>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <interfaces/adamantplugin.h>
#include <scripting/scriptsandbox.h>

#include <core.h>

PluginManager::PluginManager(CoreService *parent)
    : QObject(parent)
    , _parent(parent)
    , _networkManager(new QNetworkAccessManager(parent))
{
    // Setup Plugins Path
    _pluginPath = _parent->applicationPath();
    if (!_pluginPath.cd("plugins")) {
        _pluginPath.mkdir("plugins");
        _pluginPath.cd("plugins");
    }
    _scriptPath = _parent->applicationPath();
    if (!_scriptPath.cd("scripts")) {
        _scriptPath.mkdir("scripts");
        _scriptPath.cd("scripts");
    }
    _pluginPath.setNameFilters(QStringList({"*.dll", "*.so", "*.so.*"}));
    _pluginPath.setFilter(QDir::Files | QDir::Readable);


    _scriptPath.setNameFilters(QStringList({"*.qs"}));
    _scriptPath.setFilter(QDir::Files | QDir::Readable);

//    connect(_networkManager, &QNetworkAccessManager::finished, [this](QNetworkReply* reply) {
//        if (reply->request().originatingObject() == this) {
//            const QNetworkRequest::Attribute dataAttr = (QNetworkRequest::Attribute)((int)QNetworkRequest::User + 1);

//            QVariant vData = reply->request().attribute(dataAttr);
//            AdamantPluginInfo* data = vData.value<AdamantPluginInfo*>();
//            if (data) {
//                const QByteArray result = reply->readAll();
//                QJsonDocument doc = QJsonDocument::fromJson(result);
//                if (!doc.isObject()) {
//                    emit PluginMessage(result, DebugMessage);
//                    return;
//                }
//                if (doc.object().value("result").toBool()) {
//                    data->state = PluginState::UpToDate;
//                }
//                else {
//                    data->state = PluginState::UpdateRequired;
//                }
//                emit PluginStateChanged(data->name, data->state);
//            }
//        }
//        reply->deleteLater();
//    });
}

const QDir PluginManager::PluginsPath() const {
    return _pluginPath;
}

void PluginManager::ScanPlugins(bool initialScan) {
    if (initialScan) {
        QFileInfoList list = _pluginPath.entryInfoList();
        for (QFileInfo info : list) {
            QPluginLoader* loader = new QPluginLoader(info.absoluteFilePath(), this);
            QJsonObject metaData = loader->metaData().value("MetaData").toObject();

            const QString name = metaData.value("name").toString("Unknown");
            const QString role = metaData.value("role").toString("Unknown");

            // Ensure IID is inserted.
            metaData.insert("IID", loader->metaData().value("IID"));

            AdamantPluginInfo* data = new AdamantPluginInfo;
            data->state = PluginState::Unknown;
            data->name = name;
            data->role = role;
            data->metaData = metaData;
            data->instance = nullptr;
            data->script = nullptr;
            data->loader = nullptr;
            data->file = info;
            data->lastModified = info.lastModified();
            _pluginsByRole.insertMulti(role, data);

            emit PluginDiscovered(name);

            loader->deleteLater();
        }
    }
}

void PluginManager::VerifyPlugins() {
    emit BeginningPluginVerification();
    for (AdamantPluginInfo* data : _pluginsByRole.values()) {
        //VerifyPlugin(data);
        data->state = PluginState::UpToDate;
        emit PluginStateChanged(data->name, data->state);
    }
}

void PluginManager::VerifyPlugin(AdamantPluginInfo* plugin) {
    QNetworkRequest request;
    QString url = "http://poe.rory.io/api/v1/plugin/%1/is_latest/%2";
    QString ver = plugin->metaData.value("version").toString();
    if (plugin->name.endsWith("UI")) ver = QString();

    if (!ver.isEmpty()) ver.append("/");

    request.setUrl(QUrl(url.arg(plugin->name).arg(ver)));
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    const QNetworkRequest::Attribute dataAttr = (QNetworkRequest::Attribute)((int)QNetworkRequest::User + 1);
    request.setAttribute(dataAttr, QVariant::fromValue(plugin));
    request.setOriginatingObject(this);

    _networkManager->get(request);
}

void PluginManager::LoadAndExecuteScripts() {
    // Load plugin scripts first
    for (AdamantPluginInfo* container : _plugins) {
        if (container->script != nullptr) {
            // TODO(rory): Is invoke necessary here? Ref. Multithreading
            container->script->EvaluateProgram();
        }
    }

    LoadScripts();
}

void PluginManager::LoadScripts() {
    QFileInfoList list = _scriptPath.entryInfoList();
    for (QFileInfo info : list) {
        QFile file(info.absoluteFilePath());
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QString script = file.readAll();
            ScriptSandbox* s = AddScript(script);
            if (s) s->EvaluateProgram();
            file.close();
        }
    }
}

ScriptSandbox* PluginManager::AddScript(const QString &script, AdamantPlugin* owner) {
    ScriptSandbox* s = new ScriptSandbox(this, script, owner);
    if (s->IsValid()) {
        connect(s, &ScriptSandbox::terminating, [this, s] () {
            onScriptFinished(s);
        });
        connect(s, &ScriptSandbox::ScriptOutput, this, &PluginManager::OnScriptOutput);
        _scripts.append(s);
        return s;
    }
    s->deleteLater();
    return nullptr;
}

void PluginManager::ReloadScripts() {
    for (ScriptSandbox* script : _scripts) {
        // We only want to reload scripts that are not owned
        if (script->GetOwner() != nullptr) continue;
        script->Terminate();
    }

    LoadScripts();
}

void PluginManager::OnPluginMessage(const AdamantPlugin* plugin, QString message, QtMsgType type) const {
    if (plugin) {
        AdamantPluginInfo* data = GetPluginData(plugin);
        if (data) {
            message.prepend(QString("[%1] ").arg(data->name));
        }
    }
    emit PluginMessage(message, type);
}

void PluginManager::onScriptFinished(ScriptSandbox* s) {
    _scripts.removeAll(s);
}

void PluginManager::PreparePlugins() {
    // Ordered by load priority
    QStringList knownRoles = {"core", "service", "widget", "window"};

    QQueue<AdamantPluginInfo*> plugins;

    for (const QString role : knownRoles) {
        for (AdamantPluginInfo* data : _pluginsByRole.values(role)) {
            // Here we want to enqueue plugins to be loaded
            plugins.enqueue(data);
        }
    }

    // Clear master list
    _plugins.clear();

    // Now they should be in the correct role loading order.
    while (!plugins.isEmpty()) {
        AdamantPluginInfo* data = plugins.dequeue();

        // LOADER CODE
        data->loader = new QPluginLoader(data->file.absoluteFilePath(), this);
        bool load = data->loader->load();
        bool loaded = data->loader->isLoaded();
        QString error = data->loader->errorString();
        if (load && loaded) {
            AdamantPlugin* plugin = qobject_cast<AdamantPlugin*>(data->loader->instance());
            if (plugin != nullptr) {
                data->instance = plugin;

                qDebug() << data->name << error;

                // Now we inject the PluginManager
                InjectPluginData(data);

                // The plugin has been created successfully, so we can execute it's script (if it exists).
                const QDir dir = data->file.absoluteDir();
                QFile scriptFile(dir.filePath(data->file.fileName() + ".qs"));
                if (scriptFile.exists() && scriptFile.open(QFile::ReadOnly)) {
                    const QString script = scriptFile.readAll();
                    scriptFile.close();

                    data->script = AddScript(script, data->instance);
                }

                _plugins.append(data);
                continue;
            }
        }

        // Delete if we couldn't load
        data->loader->deleteLater();
        delete data;
        data = nullptr;
    }

    _pluginsByRole.clear();

    LoadAndExecuteScripts();
}

void PluginManager::LoadPlugins() {
    qDebug() << "Loading plugins " << _plugins.count();
    for (const AdamantPluginInfo* data : _plugins) {
        // Invoke so we can finish triggered all loads then bail.
        QMetaObject::invokeMethod(data->instance, "OnLoad", Qt::QueuedConnection);
        qDebug() << "Loaded " << data->name;
        emit PluginLoadStarted(data->name);
    }
    Finish();
}

const QJsonObject PluginManager::GetPluginMetaData(const AdamantPlugin *plugin) const {
    for (const AdamantPluginInfo* data : _plugins) {
        if (data->instance == plugin) {
            return data->loader->metaData();
        }
    }
    return QJsonObject();
}

AdamantPluginInfo* PluginManager::GetPluginData(const AdamantPlugin *plugin) const {
    for (AdamantPluginInfo* data : _plugins) {
        if (data->instance == plugin) {
            return data;
        }
    }
    return nullptr;
}

void PluginManager::Finish() {
    emit PluginLoadingFinished();
}

void PluginManager::OnScriptOutput(const QString &output) {
    for (const AdamantPluginInfo* data : _plugins) {
        if (data->instance != 0) {
            data->instance->OnScriptResult(output);
        }
    }
}

#define PLUGIN_INJECT_OBJECT(name, type, value) \
{ \
    int index = metaObject->indexOfProperty(#name); \
    if (index != -1) { \
    QMetaProperty property = metaObject->property(index); \
        if (property.isWritable()) { \
            property.write(plugin->instance, QVariant::fromValue<type>(value)); \
        } \
    } \
}

bool PluginManager::InjectPluginData(AdamantPluginInfo* plugin) {
    // Setup Injection
    // Objects added here will be injected as long as they exist in AdamantPlugin.
    const QMetaObject* metaObject = plugin->instance->metaObject();
    PLUGIN_INJECT_OBJECT(Settings, QSettings*, _parent->Settings())
    PLUGIN_INJECT_OBJECT(SensitiveSettings, QSettings*, _parent->SensitiveSettings())
    PLUGIN_INJECT_OBJECT(Core, CoreService*, _parent)

    // Setup connections
    connect(plugin->instance, &AdamantPlugin::ApplicationExit, qApp, QApplication::quit);
    connect(plugin->instance, &AdamantPlugin::Message, this, &PluginManager::OnPluginMessage);
    connect(plugin->instance, &AdamantPlugin::ReloadScripts, this, &PluginManager::ReloadScripts);

    return true;
}












