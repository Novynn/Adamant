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

const QDir PluginManager::pluginsPath() const {
    return _pluginPath;
}

void PluginManager::scanPlugins(bool initialScan) {
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

            emit pluginDiscovered(name);

            loader->deleteLater();
        }
    }
}

void PluginManager::verifyPlugins() {
    emit beginningPluginVerification();
    for (AdamantPluginInfo* data : _pluginsByRole.values()) {
        //VerifyPlugin(data);
        data->state = PluginState::UpToDate;
        emit pluginStateChanged(data->name, data->state);
    }
}

void PluginManager::verifyPlugin(AdamantPluginInfo* plugin) {
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

void PluginManager::loadAndExecuteScripts() {
    // Load plugin scripts first
    for (AdamantPluginInfo* container : _plugins) {
        if (container->script != nullptr) {
            container->script->evaluateProgram();
        }
    }

    loadScripts();
}

void PluginManager::loadScripts() {
    QFileInfoList list = _scriptPath.entryInfoList();
    for (QFileInfo info : list) {
        QFile file(info.absoluteFilePath());
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QString script = file.readAll();
            ScriptSandbox* s = addScript(file.fileName(), script);
            if (s) s->evaluateProgram();
            file.close();
        }
    }
}

ScriptSandbox* PluginManager::addScript(const QString &file, const QString &script, AdamantPlugin* owner) {
    ScriptSandbox* s = new ScriptSandbox(this, file, script, owner);
    if (s->isValid()) {
        connect(s, &ScriptSandbox::terminating, [this, s] () {
            onScriptFinished(s);
        });
        connect(s, &ScriptSandbox::scriptOutput, this, &PluginManager::onScriptOutput);
        _scripts.append(s);
        return s;
    }
    s->deleteLater();
    return nullptr;
}

void PluginManager::reloadScripts() {
    for (ScriptSandbox* script : _scripts) {
        // We only want to reload scripts that are not owned
        if (script->getOwner() != nullptr) continue;
        script->terminate();
    }

    loadScripts();
}

void PluginManager::onPluginMessage(const AdamantPlugin* plugin, QString message, QtMsgType type) const {
    if (plugin) {
        AdamantPluginInfo* data = getPluginData(plugin);
        if (data) {
            message.prepend(QString("[%1] ").arg(data->name));
        }
    }
    emit pluginMessage(message, type);
}

void PluginManager::onScriptFinished(ScriptSandbox* s) {
    _scripts.removeAll(s);
}

void PluginManager::preparePlugins() {
    // Ordered by load priority
    QStringList knownRoles = {"core", "service", "widget", "window", "utility"};

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

                // Now we inject the PluginManager
                injectPluginData(data);

                // The plugin has been created successfully, so we can execute it's script (if it exists).
                const QDir dir = data->file.absoluteDir();
                QFile scriptFile(dir.filePath(data->file.fileName() + ".qs"));
                if (scriptFile.exists() && scriptFile.open(QFile::ReadOnly)) {
                    const QString script = scriptFile.readAll();
                    scriptFile.close();

                    data->script = addScript(scriptFile.fileName(), script, data->instance);
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

    loadAndExecuteScripts();
}

void PluginManager::loadPlugins() {
    qInfo() << qPrintable(QString("Loading %1 plugin%2...").arg(_plugins.count()).arg(_plugins.count() == 1 ? "" : "s"));
    for (const AdamantPluginInfo* data : _plugins) {
        // Invoke so we can finish triggered all loads then bail.
        QMetaObject::invokeMethod(data->instance, "OnLoad", Qt::QueuedConnection);
        qInfo() << qPrintable("Loaded " + data->name);
        emit pluginLoadStarted(data->name);
    }
    finish();
}

const QJsonObject PluginManager::getPluginMetaData(const AdamantPlugin *plugin) const {
    for (const AdamantPluginInfo* data : _plugins) {
        if (data->instance == plugin) {
            return data->loader->metaData();
        }
    }
    return QJsonObject();
}

AdamantPluginInfo* PluginManager::getPluginData(const AdamantPlugin *plugin) const {
    for (AdamantPluginInfo* data : _plugins) {
        if (data->instance == plugin) {
            return data;
        }
    }
    return nullptr;
}

void PluginManager::finish() {
    emit pluginLoadingFinished();
}

void PluginManager::onScriptOutput(const QString &output) {
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

bool PluginManager::injectPluginData(AdamantPluginInfo* plugin) {
    // Setup Injection
    // Objects added here will be injected as long as they exist in AdamantPlugin.
    const QMetaObject* metaObject = plugin->instance->metaObject();

    QSettings* pluginSettings = new QSettings(pluginsPath().absoluteFilePath(plugin->file.completeBaseName() + ".ini"), QSettings::IniFormat, this);

    PLUGIN_INJECT_OBJECT(Settings, QSettings*, pluginSettings)
    PLUGIN_INJECT_OBJECT(Core, CoreService*, _parent)

    // Setup connections
    connect(plugin->instance, &AdamantPlugin::ApplicationExit, qApp, QApplication::quit);
    connect(plugin->instance, &AdamantPlugin::Message, this, &PluginManager::onPluginMessage);
    connect(plugin->instance, &AdamantPlugin::ReloadScripts, this, &PluginManager::reloadScripts);

    return true;
}












