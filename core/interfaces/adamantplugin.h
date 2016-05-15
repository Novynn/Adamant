#ifndef ADAMANTPLUGIN
#define ADAMANTPLUGIN

#include <QtPlugin>
#include <QObject>
#include <QSettings>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>

class CoreService;
class AdamantUI;

class AdamantPlugin : public QObject
{
    Q_OBJECT
public:
    virtual ~AdamantPlugin() {}
    Q_PROPERTY(QSettings* Settings MEMBER _settings)
    Q_PROPERTY(QSettings* SensitiveSettings MEMBER _sensitiveSettings)
    Q_PROPERTY(CoreService* Core MEMBER _core)

    void Log(QString m, QtMsgType t = QtDebugMsg) const {
        emit Message(this, m, t);
    }

    void RequestApplicationExit() const {
        emit ApplicationExit();
    }
public slots:
    virtual void OnLoad() {}
    virtual void setupEngine(QScriptEngine* engine, QScriptValue* plugin) {Q_UNUSED(engine); Q_UNUSED(plugin);}
    virtual void OnScriptResult(const QString &result) {Q_UNUSED(result);}

    virtual const QString GetPluginScript() {return QString();}
protected:
    QSettings* Settings() const {
        return _settings;
    }

    QSettings* SensitiveSettings() const {
        return _sensitiveSettings;
    }

    CoreService* Core() const {
        return _core;
    }
signals:
    void ApplicationExit() const;
    void ReloadScripts();

    void Message(const AdamantPlugin*, QString, QtMsgType) const;
private:
    QSettings* _settings;
    QSettings* _sensitiveSettings;
    CoreService* _core;
};

Q_DECLARE_INTERFACE(AdamantPlugin, "adamant.plugin/1.0")
Q_DECLARE_OPAQUE_POINTER(AdamantPlugin*)
Q_DECLARE_METATYPE(AdamantPlugin*)

#endif // ADAMANTPLUGIN
