#ifndef GLOBALHOTKEYPLUGIN_H
#define GLOBALHOTKEYPLUGIN_H

#include "adamantplugin.h"
#include "qxt/qxtglobalshortcut.h"
#include <memory>
class PluginManager;

class GlobalHotkeyPlugin : public AdamantPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "com.adamant.plugin.globalhotkey" FILE "globalhotkey.json")
    Q_INTERFACES(AdamantPlugin)

public:
    static QScriptValue Listen(QScriptContext *context , QScriptEngine *engine, void *arg) {
        GlobalHotkeyPlugin* instance = qobject_cast<GlobalHotkeyPlugin*>((QObject*)arg);
        if (instance) {
            if (context->argumentCount() != 2) {
                return context->throwError(QScriptContext::SyntaxError, "Listen takes 2 arguments");
            }
            QString keySequence = context->argument(0).toString();
            QScriptValue callback = context->argument(1);

            if (!callback.isFunction()) {
                return context->throwError(QScriptContext::SyntaxError, "Listen's 2nd argument is a function");
            }

            QxtGlobalShortcut* shortcut = nullptr;
            if (!instance->shortcuts.contains(keySequence)) {
                shortcut = new QxtGlobalShortcut(QKeySequence(keySequence));
                shortcut->setEnabled(true);
                instance->shortcuts.insert(keySequence, shortcut);
            }
            else {
                shortcut = instance->shortcuts.value(keySequence);
            }

            connect(shortcut, &QxtGlobalShortcut::activated, [callback]() mutable {
                callback.call();
            });
            return engine->undefinedValue();
        }
        return context->throwError(QScriptContext::ReferenceError, "could not find script object");
    }
public slots:
    void SetupEngine(QScriptEngine* engine, QScriptValue* plugin) {
        plugin->setProperty("Listen", engine->newFunction(GlobalHotkeyPlugin::Listen, this));
    }
private:
    QMap<QString, QxtGlobalShortcut*> shortcuts;
};

#endif // GLOBALHOTKEYPLUGIN_H

