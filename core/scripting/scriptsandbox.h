#ifndef SCRIPTSANDBOX_H
#define SCRIPTSANDBOX_H

#include <QObject>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptProgram>
#include <QtScript/QScriptValue>
#include <QtScriptTools/QScriptEngineDebugger>
#include <QtScript>

class PluginManager;
class AdamantPlugin;

class ScriptSandboxTemp : public QObject {
    Q_OBJECT
public:
    QScriptValue thisObject;
    QScriptValue value;

    Q_INVOKABLE void Execute() {
        value.call(thisObject);
        deleteLater();
    }
};

class ScriptSandbox : public QObject
{
    Q_OBJECT
public:
    explicit ScriptSandbox(const PluginManager *manager, const QString &script, AdamantPlugin *owner = 0);
    ~ScriptSandbox();

    QScriptValue AddGlobalObject(const QString &name, QObject *object);
    Q_INVOKABLE bool IsValid() const {
        return _errorString.isEmpty();
    }
    Q_INVOKABLE const QString ErrorString() const {
        return _errorString;
    }

    Q_PROPERTY(AdamantPlugin* Owner MEMBER _owner READ GetOwner)

    AdamantPlugin* GetOwner() const {
        return _owner;
    }

    bool AddLine(const QString &script);
public slots:
    void EvaluateProgram();
    void Terminate();
signals:
    void terminating();
    void ScriptOutput(const QString &output);
private:
    const PluginManager* _manager;
    QString _script;
    AdamantPlugin* _owner;

    QScriptProgram _program;
    QScriptEngine _engine;
    QScriptEngineDebugger _engineDebugger;

    QString _errorString;
    static QScriptValue runFunc(QScriptContext *context, QScriptEngine *engine, void *arg);
    static QScriptValue importFunc(QScriptContext *context, QScriptEngine *engine, void *arg);
    static QScriptValue printFunc(QScriptContext *context, QScriptEngine *engine, void *arg);
};

#endif // SCRIPTSANDBOX_H
