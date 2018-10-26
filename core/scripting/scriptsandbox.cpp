#include "scriptsandbox.h"
#include <interfaces/adamantplugin.h>
#include <pluginmanager.h>
#include <QRegularExpression>
#include <core.h>
#include <session/sessionrequest.h>
#include <ui/ui.h>
#include <items/itemmanager.h>

QScriptValue ScriptSandbox::runFunc(QScriptContext *context , QScriptEngine *engine, void *arg) {
    ScriptSandbox* instance = qobject_cast<ScriptSandbox*>((QObject*)arg);
    if (instance) {
        QScriptValue val = context->argument(0);

        ScriptSandboxTemp* temp = new ScriptSandboxTemp;
        temp->value = val;
        temp->thisObject = context->thisObject();

        // Detect thread
        Qt::ConnectionType type = Qt::AutoConnection;
        if (QThread::currentThread() != qApp->thread()) {
            type = Qt::BlockingQueuedConnection;
            temp->setParent(0);
            temp->moveToThread(qApp->thread());
        }
        QMetaObject::invokeMethod(temp, "execute", type);
        return engine->undefinedValue();
    }
    return context->throwError(QScriptContext::ReferenceError, "could not find script object");
}

QScriptValue ScriptSandbox::importFunc(QScriptContext *context , QScriptEngine *engine, void* arg) {
    ScriptSandbox* instance = qobject_cast<ScriptSandbox*>((QObject*)arg);
    if (instance) {
        if (context->argumentCount() != 1) {
            return context->throwError(QScriptContext::SyntaxError, "import takes 1 argument");
        }
        QString import = context->argument(0).toString();
        AdamantPlugin* plugin = instance->_manager->getPluginByIID(import);
        if (plugin) {
            QScriptValue value = engine->newQObject(plugin, QScriptEngine::QtOwnership, QScriptEngine::ExcludeDeleteLater);
            plugin->setupEngine(engine, &value);
            return value;
        }
        return context->throwError(QScriptContext::ReferenceError, "could not find plugin [" + import + "]");
    }
    return context->throwError(QScriptContext::ReferenceError, "could not find script object");
}

QScriptValue ScriptSandbox::printFunc(QScriptContext *context , QScriptEngine *engine, void* arg) {
    ScriptSandbox* instance = qobject_cast<ScriptSandbox*>((QObject*)arg);
    if (instance) {
        QtMsgType type = QtDebugMsg;
        QString result;

        if (context->argumentCount() > 0) {
            QString first = context->argument(0).toString();
            if      (first.compare("debug",     Qt::CaseInsensitive) == 0) type = QtDebugMsg;
            else if (first.compare("info",      Qt::CaseInsensitive) == 0) type = QtInfoMsg;
            else if (first.compare("warning",   Qt::CaseInsensitive) == 0) type = QtWarningMsg;
            else if (first.compare("error",     Qt::CaseInsensitive) == 0) type = QtCriticalMsg;
            else if (first.compare("fatal",     Qt::CaseInsensitive) == 0) type = QtFatalMsg;
            else {
                result = first;
            }
        }

        for (int i = 1; i < context->argumentCount(); i++) {
            if (!result.isEmpty()) result.append(" ");
            result.append(context->argument(i).toString());
        }

        // If we're owned by a Plugin, channel the message through
        if (instance->_owner != nullptr) {
            instance->_owner->Log(result, type);
        }
        else {
            instance->_manager->onPluginMessage(nullptr, result, type);
        }

        return engine->undefinedValue();
    }
    return context->throwError(QScriptContext::ReferenceError, "could not find script object");
}

ScriptSandbox::ScriptSandbox(const PluginManager *parent, const QString &file, const QString &script, AdamantPlugin *owner)
    : QObject()
    , _manager(parent)
    , _script(script)
    , _owner(owner)
    , _engine()
    , _engineDebugger()
    , _errorString()
{
    _engineDebugger.attachTo(&_engine);
    _program = QScriptProgram(_script, file);

    // Register MetaTypes, sometimes Q_DECL doesn't seem to cut it...
    qRegisterMetaType<Session*>("Session");
    qRegisterMetaType<Session::Request*>("Session::Request");
    qRegisterMetaType<AdamantUI*>("AdamantUI");
    qRegisterMetaType<ItemManager*>("ItemManager");
    qRegisterMetaType<AdamantUI::ApplicationTheme>("ApplicationTheme");
//    qScriptRegisterMetaType<AdamantUI::ApplicationTheme>(&_engine, [](QScriptEngine *engine, const AdamantUI::ApplicationTheme &t) -> QScriptValue {
//        return QScriptValue(static_cast<int>(t));
//    }, [](const QScriptValue &obj, AdamantUI::ApplicationTheme &t) {
//        t = static_cast<AdamantUI::ApplicationTheme>(obj.toInt32());
//    });


    QScriptSyntaxCheckResult syntax = _engine.checkSyntax(_script);
    if (syntax.state() != QScriptSyntaxCheckResult::Valid){
        _errorString = syntax.errorMessage();
    }
    else {
        // Default Globals
        addGlobalObject("manager", (QObject*)_manager);
        addGlobalObject("script", this);
        addGlobalObject("core", _manager->core());
        addGlobalObject("app", qApp);
    }

    {
        QScriptValue func = _engine.newFunction(ScriptSandbox::runFunc, this);
        _engine.globalObject().setProperty("run", func);
    }

    {
        QScriptValue func = _engine.newFunction(ScriptSandbox::importFunc, this);
        _engine.globalObject().setProperty("using", func);
    }

    {
        QScriptValue func = _engine.newFunction(ScriptSandbox::printFunc, this);
        _engine.globalObject().setProperty("print", func);
    }
}

void ScriptSandbox::setup() {
    const AdamantUI* ui = _manager->core()->getInterface();
    _engineDebugger.standardWindow()->setPalette(ui->getLightPalette());
}

ScriptSandbox::~ScriptSandbox() {
    if (_engine.isEvaluating()) {
        _engine.abortEvaluation();
    }
}

QScriptValue ScriptSandbox::addGlobalObject(const QString &name, QObject* object) {
    QScriptValue value = _engine.newQObject(object, QScriptEngine::QtOwnership, QScriptEngine::ExcludeDeleteLater);
    _engine.globalObject().setProperty(name, value);
    return value;
}

void ScriptSandbox::evaluateProgram() {
    QScriptValue val = _engine.evaluate(_program);
    if (val.isError()) {
    }
    else if (_engine.hasUncaughtException()) {
        int line = _engine.uncaughtExceptionLineNumber();
        Q_UNUSED(line)
    }
    else if (!val.isUndefined()) {
    }
    _script = QString();
}

bool ScriptSandbox::addLine(const QString &script) {
    _script.append(script).append("\n");
    if (_engine.canEvaluate(_script)) {
        QScriptValue val = _engine.evaluate(_script);
        if (val.isError()) {
            emit scriptOutput(val.toString());
        }
        else if (_engine.hasUncaughtException()) {
            int line = _engine.uncaughtExceptionLineNumber();
            emit scriptOutput("Uncaught exception at line " + QString::number(line) + ":" + val.toString());
        }
        else if (!val.isUndefined()) {
            emit scriptOutput(val.toString());
        }
        _script = QString();
        return true;
    }
    return false;
}

void ScriptSandbox::terminate()
{
    // Give the script a chance to clean up...
    emit terminating();
    deleteLater();
}
