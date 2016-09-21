#include "adamantskilltreeplugin.h"
#include "skilltreeviewer.h"
#include <core.h>
#include <ui/ui.h>

void AdamantSkillTreePlugin::OnLoad() {
    {
        _viewer = new SkillTreeViewer(this);
        Core()->getInterface()->registerPluginPage(this, QIcon(":/icons/dark/tree.png"),
                                                       "Skill Tree", "Plan and explore Path of Exile's Passive Skill Tree",
                                                       _viewer);
    }
}

void AdamantSkillTreePlugin::setupEngine(QScriptEngine* engine, QScriptValue* plugin) {
    Q_UNUSED(engine)
    Q_UNUSED(plugin)
}
