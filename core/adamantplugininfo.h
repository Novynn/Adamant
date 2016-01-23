#ifndef ADAMANTPLUGININFO
#define ADAMANTPLUGININFO

#include <QObject>
#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include <QFileInfo>

class QPluginLoader;
class AdamantPlugin;
class ScriptSandbox;

enum class PluginState {
    Unknown,
    Unverified,
    UpdateRequired,
    UpToDate
};

struct AdamantPluginInfo {
    Q_GADGET
public:
    PluginState state;
    QString name;
    QString role;
    QJsonObject metaData;

    QFileInfo file;
    QDateTime lastModified;

    AdamantPlugin* instance;
    ScriptSandbox* script;
    QPluginLoader* loader;
};
Q_DECLARE_METATYPE(AdamantPluginInfo*)
Q_DECLARE_METATYPE(PluginState)

#endif // ADAMANTPLUGININFO

