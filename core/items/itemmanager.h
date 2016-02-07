#ifndef ITEMMANAGER_H
#define ITEMMANAGER_H

#include <core_global.h>
#include <QObject>
#include <QJsonDocument>
#include <QVariant>
#include <QRegularExpression>

class CoreService;
class StashItemLocation;
class ItemManagerInstance;

struct ItemManagerInstanceTab {
    int tabIndex;
    StashItemLocation* location;
    ItemManagerInstance* instance;
    bool error;
};

struct ItemManagerInstance {
    QString league;
    QRegularExpression filter;
    bool firstTabReceived;
    int totalTabs;
    int receivedTabs;
    QMap<int, ItemManagerInstanceTab*> tabs;
};

class CORE_EXTERN ItemManager : public QObject
{
    Q_OBJECT
public:
    explicit ItemManager(CoreService *parent);

    Q_INVOKABLE void getCharacter(const QString &characterName) {
        Q_UNUSED(characterName)
    }

    Q_INVOKABLE void fetchStashTabs(const QString &league, const QString &filter = QString());
    Q_INVOKABLE QList<StashItemLocation*> GetStashTabs(const QString &league) {
        QList<StashItemLocation*> stash;
        if (_currentInstances.contains(league)) {
            for (ItemManagerInstanceTab* tab : _currentInstances.value(league)->tabs.values()) {
                stash.append(tab->location);
            }
        }
        return stash;
    }

signals:
    void onCharacterUpdateAvailable(QString characterName);
    void onStashTabUpdateBegin(QString league);
    void onStashTabUpdateProgress(QString league, int received, int total);
    void onStashTabUpdateAvailable(QString league);
public slots:
    void updateStashTabs(){}
    void updateCharacters(){}
    void updateCharacter(QString characterName){Q_UNUSED(characterName)}
private slots:
    void onStashTabResult(QString league, QByteArray json, QVariant data);
private:
    CoreService* _core;

    QHash<QString, ItemManagerInstance*> _currentInstances;
    QHash<QString, ItemManagerInstance*> _fetchingInstances;
};
Q_DECLARE_METATYPE(ItemManager*)
Q_DECLARE_METATYPE(ItemManagerInstance*)
Q_DECLARE_METATYPE(ItemManagerInstanceTab*)

#endif // ITEMMANAGER_H
