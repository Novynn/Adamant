#ifndef ITEMMANAGER_H
#define ITEMMANAGER_H

#include <core_global.h>
#include <QObject>
#include <QJsonDocument>
#include <QVariant>
#include <QRegularExpression>
#include <QQueue>

class CoreService;
class ItemLocation;
class StashItemLocation;
class CharacterItemLocation;
class ItemManagerInstance;
class ItemManager;

struct ItemManagerInstanceTab {
    int tabIndex;
    StashItemLocation* location;
    ItemManagerInstance* instance;
    bool error;
};

struct ItemManagerInstance {
    QString accountName;
    bool throttled;
    ItemManager* manager;

    struct Stash {
        QString league;
        int totalTabs;
        int receivedTabs;
        QRegularExpression filter;
        bool firstTabReceived;
        QMap<int, ItemManagerInstanceTab*> tabs;
    };

    struct Character {
        QString name;
        CharacterItemLocation* location;
        QString classType;
        int level;
    };

    union {
        Stash* stash;
        Character* character;
    };
};

// Or we could just use polymorphism... But this is fun!
struct Fetchable {
    enum {
        Instance,
        Character,
        Tab
    } type;

    union {
        void* ptr;
        ItemManagerInstance* instance;
        ItemManagerInstanceTab* tab;
    };
};

class CORE_EXTERN ItemManager : public QObject
{
    Q_OBJECT
public:
    explicit ItemManager(CoreService *parent);

    Q_INVOKABLE void fetchCharacterItems(const QString &characterName, const QString &classType, int level);
    Q_INVOKABLE void fetchStashTabs(const QString &league, const QString &filter = QString());

    Q_INVOKABLE QList<StashItemLocation*> getStashTabs(const QString &league) {
        QList<StashItemLocation*> stash;
        if (_currentInstances.contains(league)) {
            for (ItemManagerInstanceTab* tab : _currentInstances.value(league)->stash->tabs.values()) {
                stash.append(tab->location);
            }
        }
        return stash;
    }

    Q_INVOKABLE CharacterItemLocation* getCharacterItems(const QString &character) {
        return _currentInstances.value("CHARACTER_" + character, nullptr)->character->location;
    }

protected:
    void fetchFirstStashTab(ItemManagerInstance* instance);
    void queueStashTab(ItemManagerInstanceTab* tab);
    void queueCharacter(ItemManagerInstance* instance);
    static void beginFetch();
    static void pruneHistory();
    static QQueue<Fetchable*> _fetchQueue;
    static QMap<Fetchable*, QDateTime> _fetchHistory;
    static bool _fetchWaiting;
    static const int RequestsPerPeriod;
    static const int RequestPeriodMSecs;
    static const int RequestPeriodWait;
    static void updateFetchable(Fetchable* fetchable);
    static void updateFetchable(ItemManagerInstance* instance, int type = Fetchable::Instance);
    static void updateFetchable(ItemManagerInstanceTab* tab);
    void fetchStashTab(ItemManagerInstanceTab* tab);
    void fetchCharacter(ItemManagerInstance* instance);
    void queueFirstStashTab(ItemManagerInstance* instance);
signals:
    void onCharacterUpdateAvailable(QString characterName);
    void onStashTabUpdateBegin(QString league);
    void onStashTabUpdateProgress(QString league, int received, int total, bool throttled);
    void onStashTabUpdateAvailable(QString league);
public slots:
    void updateStashTabs(){}
    void updateCharacters(){}
    void updateCharacter(QString characterName){Q_UNUSED(characterName)}
private slots:
    void onStashTabResult(QString league, QByteArray json, QVariant data);
    void onCharacterItemsResult(QString character, QByteArray json, QVariant data);
private:
    CoreService* _core;

    QHash<QString, ItemManagerInstance*> _currentInstances;
    QHash<QString, ItemManagerInstance*> _fetchingInstances;
};
Q_DECLARE_METATYPE(ItemManager*)
Q_DECLARE_METATYPE(ItemManagerInstance*)
Q_DECLARE_METATYPE(ItemManagerInstanceTab*)

#endif // ITEMMANAGER_H
