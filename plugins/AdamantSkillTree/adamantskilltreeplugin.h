#ifndef ADAMANTSKILLTREE_H
#define ADAMANTSKILLTREE_H

#include "adamantplugin.h"
class SkillTreeViewer;

class AdamantSkillTreePlugin : public AdamantPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "adamant.skilltree" FILE "adamantskilltree.json")
    Q_INTERFACES(AdamantPlugin)

public:
    AdamantSkillTreePlugin()
        : _viewer(nullptr) {
    }
public slots:
    void OnLoad();
private:
    SkillTreeViewer* _viewer;
public slots:
    void setupEngine(QScriptEngine* engine, QScriptValue* plugin);
};

#endif // ADAMANTSKILLTREE_H
