#ifndef ITEMMANAGER_H
#define ITEMMANAGER_H

#include <core_global.h>
#include <QObject>
#include <QJsonDocument>
#include <QVariant>
#include <QRegularExpression>
#include <QQueue>
#include <QDir>

class CoreService;
class ItemLocation;
class StashItemLocation;
class CharacterItemLocation;
class ItemManagerInstance;
class ItemManager;

struct ItemManagerInstance {
    QString accountName;
    bool throttled;
    bool error;
    ItemManager* manager;

    enum class Type {
        StashTab,
        Character
    } type;

    struct StashTab {
        QString league;
        QString tabId;
        int tabIndex;
    };

    struct Character {
        QString league;
        QString name;
        QString classType;
        int level;
    };

    ItemLocation* location;
    union {
        StashTab* tab;
        Character* character;
    };
};

class CORE_EXTERN ItemManager : public QObject
{
    Q_OBJECT
public:
    explicit ItemManager(CoreService *parent);

    Q_INVOKABLE void fetchCharacterItems(const QString &characterName, const QString &classType, int level);
    Q_INVOKABLE void fetchStashTab(const QString &league, QString id);

    Q_INVOKABLE QList<StashItemLocation*> getStashTabs(const QString &league);
    Q_INVOKABLE StashItemLocation* getStashTab(const QString& tabId);

    Q_INVOKABLE CharacterItemLocation* getCharacterItems(const QString &character);

    Q_INVOKABLE void saveStash(QString league);
    Q_INVOKABLE void printIndexMap(QString league = QString());

    static QDir StashDataDir(QString league);
protected:
    void queueStashTab(ItemManagerInstance* instance);
    void queueCharacter(ItemManagerInstance* instance);

    qint32 getStashIndexById(const QString& league, const QString &id);

    static void beginFetch();
    static void pruneHistory();
    static QQueue<ItemManagerInstance*> _fetchQueue;
    static QMap<ItemManagerInstance*, QDateTime> _fetchHistory;
    static bool _fetchWaiting;
    static const int RequestsPerPeriod;
    static const int RequestPeriodMSecs;
    static const int RequestPeriodWait;
    static void updateFetchable(ItemManagerInstance* instance);
    void fetchStashTab(ItemManagerInstance* instance);
    void fetchCharacter(ItemManagerInstance* instance);
signals:
    void onCharacterUpdateProgress(QString characterName, bool throttled);
    void onCharacterUpdate(QString characterName);
    void onStashTabIndexMapUpdate(QString league);
    void onStashTabUpdateProgress(QString league, QString tabId, bool throttled);
    void onStashTabUpdate(QString league, QString tabId);
public slots:
    void updateStashTabs(){}
    void updateCharacters(){}
    void updateCharacter(QString characterName){Q_UNUSED(characterName)}
private slots:
    void onStashTabResult(QString league, QByteArray json, QVariant data);
    void onCharacterItemsResult(QString character, QByteArray json, QVariant data);
private:
    CoreService* _core;

    QHash<QString, ItemManagerInstance*> _currentTabInstances;
    QHash<QString, ItemManagerInstance*> _currentCharacterInstances;
    QHash<QString, ItemManagerInstance*> _fetchingTabInstances;
    QHash<QString, ItemManagerInstance*> _fetchingCharacterInstances;

    QHash<QString, QMap<QString, quint32>> _tabIndexMap;
};
Q_DECLARE_METATYPE(ItemManager*)
Q_DECLARE_METATYPE(ItemManagerInstance*)

#endif // ITEMMANAGER_H
