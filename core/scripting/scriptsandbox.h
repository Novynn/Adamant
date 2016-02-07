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

    Q_INVOKABLE void execute() {
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

    QScriptValue addGlobalObject(const QString &name, QObject *object);
    Q_INVOKABLE bool isValid() const {
        return _errorString.isEmpty();
    }
    Q_INVOKABLE const QString errorString() const {
        return _errorString;
    }

    Q_PROPERTY(AdamantPlugin* Owner MEMBER _owner READ getOwner)

    AdamantPlugin* getOwner() const {
        return _owner;
    }

    QScriptEngine* engine() {
        return &_engine;
    }

    bool addLine(const QString &script);
public slots:
    void evaluateProgram();
    void terminate();
signals:
    void terminating();
    void scriptOutput(const QString &output);
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
