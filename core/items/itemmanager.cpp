#include "itemmanager.h"
#include "core.h"
#include "stashitemlocation.h"
#include "item.h"
#include "characteritemlocation.h"
#include <QTimer>
#include <session/sessionrequest.h>

QQueue<ItemManagerInstance*> ItemManager::_fetchQueue;
QMap<ItemManagerInstance*, QDateTime> ItemManager::_fetchHistory;
bool ItemManager::_fetchWaiting = false;
const int ItemManager::RequestsPerPeriod = 45;
const int ItemManager::RequestPeriodMSecs = 60000;
const int ItemManager::RequestPeriodWait = 5000;

ItemManager::ItemManager(CoreService *parent)
    : QObject(parent)
    , _core(parent)
{
    connect(_core->session(), &Session::Request::accountStashTabs, this, &ItemManager::onStashTabResult);
    connect(_core->session(), &Session::Request::accountCharacterItems, this, &ItemManager::onCharacterItemsResult);
}

void ItemManager::fetchCharacterItems(const QString& characterName, const QString &classType, int level) {
    const QString key = "CHARACTER_" + characterName;
    if (_fetchingCharacterInstances.contains(key)) {
        qDebug() << "Already fetching " << characterName;
        return;
    }
    const QString accountName = _core->session()->accountName();
    if (accountName.isEmpty()) {
        qDebug() << "Account Name is empty!!!";
        return;
    }

    ItemManagerInstance* instance = new ItemManagerInstance;
    instance->accountName = accountName;
    instance->manager = this;
    instance->throttled = false;
    instance->sent = false;
    instance->location = nullptr;
    instance->type = ItemManagerInstance::Type::Character;
    instance->character = new ItemManagerInstance::Character;
    instance->character->name = characterName;
    instance->character->classType = classType;
    instance->character->level = level;

    _fetchingCharacterInstances.insert(key, instance);

    queueCharacter(instance);
    beginFetch();
}

void ItemManager::fetchStashTab(const QString& league, QString id) {
    if (id.isEmpty()) {
        id = "index_" + league;
    }
    if (_fetchingTabInstances.contains(id)) {
        qDebug() << "Already fetching " << id;
        return;
    }
    const QString accountName = _core->session()->accountName();
    if (accountName.isEmpty()) {
        qDebug() << "Account Name is empty!!!";
        return;
    }

    ItemManagerInstance* instance = _currentTabInstances.value(id, nullptr);
    if (instance == nullptr) {
        instance = new ItemManagerInstance;
        instance->accountName = accountName;
        instance->manager = this;
        instance->throttled = false;
        instance->sent = false;
        instance->location = nullptr;
        instance->type = ItemManagerInstance::Type::StashTab;
        instance->tab = new ItemManagerInstance::StashTab;
        instance->tab->league = league;
        instance->tab->tabId = id;
        instance->tab->tabIndex = getStashIndexById(league, id);
    }

    _fetchingTabInstances.insert(id, instance);
    _currentTabInstances.insert(id, instance);

    queueStashTab(instance);
}

QList<StashItemLocation*> ItemManager::getStashTabs(const QString& league) {
    QList<StashItemLocation*> stash;
    for (ItemManagerInstance* instance : _currentTabInstances.values()) {
        if (instance->tab->league == league && instance->location != nullptr)
            stash.append(dynamic_cast<StashItemLocation*>(instance->location));
    }
    return stash;
}

StashItemLocation* ItemManager::getStashTab(const QString &tabId) {
    ItemManagerInstance* instance = _currentTabInstances.value(tabId, nullptr);
    if (instance) return dynamic_cast<StashItemLocation*>(instance->location);
    return nullptr;
}

CharacterItemLocation*ItemManager::getCharacterItems(const QString& character) {
    return dynamic_cast<CharacterItemLocation*>(_currentCharacterInstances.value("CHARACTER_" + character, nullptr)->location);
}

QDir ItemManager::StashDataDir(QString league) {
    QDir dataDir = CoreService::dataPath();
    league = league.toLower();
    QString folder = QString("%1_cache").arg(league);
    if (!dataDir.cd(folder)) {
        dataDir.mkdir(folder);
        dataDir.cd(folder);
    }
    return dataDir;
}

void ItemManager::saveStash(ItemManagerInstance* instance) {
    QDir dataDir = ItemManager::StashDataDir(instance->tab->league);

    QString filePath = dataDir.absoluteFilePath(QString("%1.tab").arg(instance->tab->tabId));
    QJsonDocument doc(instance->location->toJson());
    const QByteArray data = doc.toJson(QJsonDocument::Indented);

    QFile file(filePath);
    if (file.open(QFile::Text | QFile::WriteOnly)) {
        file.write(data);
        file.close();
    }
}

void ItemManager::saveStash(const QString &league, const QString &tabId) {
    ItemManagerInstance* instance = _currentTabInstances.value(tabId, nullptr);
    if (instance != nullptr && instance->tab->league == league){
        saveStash(instance);
    }
}

void ItemManager::printIndexMap(QString league) {
    QStringList leagues;
    if (league.isEmpty())
        leagues << _tabIndexMap.uniqueKeys();
    else
        leagues << league;

    for (const QString &l : leagues) {
        for (const QString &id : _tabIndexMap[l].keys()) {
            _core->loggedMessage(QString("%2 #%3: %1").arg(id).arg(l).arg(_tabIndexMap[l].value(id, -1)), QtInfoMsg);
        }
    }
}

void ItemManager::queueCharacter(ItemManagerInstance* instance) {
    ItemManager::_fetchQueue.enqueue(instance);
}

qint32 ItemManager::getStashIndexById(const QString &league, const QString& id) {
    if (id.isEmpty()) return -1;
    return _tabIndexMap[league].value(id, -1);
}

void ItemManager::queueStashTab(ItemManagerInstance* instance) {
    ItemManager::_fetchQueue.enqueue(instance);
    ItemManager::beginFetch();
    if (ItemManager::_fetchWaiting) {
        emit onStashTabUpdateProgress(instance->tab->league, instance->tab->tabId, instance->throttled = true);
    }
}

void ItemManager::fetchStashTab(ItemManagerInstance *instance) {
    QString data = instance->tab->tabId;
    if (data.isEmpty()) {
        data = "index_" + instance->tab->league;
    }
    _core->session()->fetchAccountStashTabs(instance->accountName, instance->tab->league,
                                            (instance->tab->tabIndex == -1) ? 0 : instance->tab->tabIndex, true,
                                            data);
    instance->sent = true;
}

void ItemManager::fetchCharacter(ItemManagerInstance* instance) {
    _core->session()->fetchAccountCharacterItems(instance->accountName, instance->character->name);
    instance->sent = true;
}

void ItemManager::beginFetch() {
    if (ItemManager::_fetchWaiting) return;

    // Let's get to work!
    pruneHistory();
    // Only fetch tabs if we haven't gotten 45 in the last minute (this is how it works on GGG's end).
    if (_fetchHistory.count() < ItemManager::RequestsPerPeriod) {
        while (!ItemManager::_fetchQueue.isEmpty()) {
            ItemManagerInstance* instance = ItemManager::_fetchQueue.dequeue();
            ItemManager::_fetchHistory.insert(instance, QDateTime());

            ItemManager* manager = instance->manager;
            if (instance->type == ItemManagerInstance::Type::StashTab) {
                manager->fetchStashTab(instance);
                emit manager->onStashTabUpdateProgress(instance->tab->league, instance->tab->tabId, false);
            }
            else if (instance->type == ItemManagerInstance::Type::Character) {
                manager->fetchCharacter(instance);
                emit manager->onCharacterUpdateProgress(instance->character->name, false);
            }

            if (ItemManager::_fetchHistory.count() >= ItemManager::RequestsPerPeriod) {
                qInfo() << "Stash tab fetching has been throttled.";
                for (auto i : manager->_fetchingTabInstances.values()) {
                    if (i->sent) continue;
                    i->throttled = true;
                    emit manager->onStashTabUpdateProgress(i->tab->league, i->tab->tabId, i->throttled);
                }
                for (auto i : manager->_fetchingCharacterInstances.values()) {
                    if (i->sent) continue;
                    i->throttled = true;
                    emit manager->onCharacterUpdateProgress(i->character->name, i->throttled);
                }
                // Throttle!
                break;
            }
            instance->throttled = false;
        }
    }

    if (!_fetchQueue.isEmpty()) {
        QTimer::singleShot(ItemManager::RequestPeriodWait, [] {
            ItemManager::_fetchWaiting = false;
            ItemManager::beginFetch();
        });
        ItemManager::_fetchWaiting = true;
    }
}

void ItemManager::pruneHistory() {
    int start = _fetchHistory.count();
    QDateTime now = QDateTime::currentDateTime();
    QMap<ItemManagerInstance*, QDateTime> newHistory;
    while (!ItemManager::_fetchHistory.isEmpty()) {
        auto fetchable = ItemManager::_fetchHistory.firstKey();
        QDateTime date = ItemManager::_fetchHistory.take(fetchable);
        if (date.isNull() || date.msecsTo(now) < ItemManager::RequestPeriodMSecs) {
            newHistory.insert(fetchable, date);
        }
    }
    int removed = start - newHistory.size();
    ItemManager::_fetchHistory = newHistory;
    if (removed) {
        // qInfo() << qPrintable("History pruning complete: " + QString::number(removed) + (removed == 1 ? " entry" : " entries") + " deleted.");
    }
}

void ItemManager::updateFetchable(ItemManagerInstance* instance) {
    ItemManager::_fetchHistory.remove(instance);
    ItemManager::_fetchHistory.insert(instance, QDateTime::currentDateTime());
}

void ItemManager::onStashTabResult(QString league, QByteArray json, QVariant data) {
    QString key = data.toString();

    ItemManagerInstance* instance = _fetchingTabInstances.value(key, nullptr);
    if (!instance) {
        qDebug() << "Failed to fetch instance for " << league << key;
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(json);
    bool error = false;
    if (doc.isEmpty() || !doc.isObject() || doc.object().contains("error")) {
        qDebug() << "Failed to retrieve stash tabs (throttled?)";
        // TODO(rory): Handle throttling
        // {"error":{"message":"You are requesting your stash too frequently. Please try again later."}}
        qDebug() << qPrintable(json);

        // We failed to get the first tab (uh oh).
        qWarning() << qPrintable("Failed to get the first tab.");
        error = true;
    }

    ItemManager::updateFetchable(instance);

    int foundIndex = -1;

    QJsonObject root = doc.object();
    QJsonArray tabs = root.value("tabs").toArray();

    QList<ItemManagerInstance*> currentInstances;
    for (ItemManagerInstance* i : _currentTabInstances.values()) {
        if (i->tab->league == league && i != instance) {
            currentInstances.append(i);
        }
    }

    _tabIndexMap[league].clear();
    for (QJsonValue tabVal : tabs) {
        QJsonObject tab = tabVal.toObject();
        // Check hidden flag
        if (tab.value("hidden").toBool()) continue;
        QString id = tab.value("id").toString();
        int index = tab.value("i").toInt();

        // Update league tab map
        _tabIndexMap[league].insert(id, index);

        // Update tab info
        ItemManagerInstance* currentInstance = nullptr;
        StashItemLocation* current = nullptr;

        if ((currentInstance = _currentTabInstances.value(id, nullptr)) != nullptr) {
            if ((current = dynamic_cast<StashItemLocation*>(currentInstance->location)) != nullptr) {
                current->update(tab);
            }
            else {
                currentInstance->location = new StashItemLocation(tab);
                currentInstance->location->setState(ItemLocation::Loading);
            }
            currentInstances.removeOne(currentInstance);
        }
        else {
            if (id == instance->tab->tabId) {
                currentInstance = instance;
            }
            else {
                // Setup new instance
                currentInstance = new ItemManagerInstance;
                currentInstance->accountName = instance->accountName;
                currentInstance->manager = this;
                currentInstance->throttled = false;
                currentInstance->location = new StashItemLocation(tab);
                currentInstance->type = ItemManagerInstance::Type::StashTab;
                currentInstance->tab = new ItemManagerInstance::StashTab;
                currentInstance->tab->tabId = id;
                currentInstance->tab->tabIndex = index;
                currentInstance->tab->league = league;
            }
            _currentTabInstances[id] = currentInstance;
        }

        if (id == instance->tab->tabId) {
            foundIndex = index;
        }
    }

    // Clear non-existant tabs
    for (ItemManagerInstance* i : currentInstances) {
        auto i2 = _currentTabInstances.take(i->tab->tabId);
        Q_ASSERT(i == i2 && i2 != 0);
    }

    emit onStashTabIndexMapUpdate(league);

    // Actually delete
    while (!currentInstances.isEmpty()) {
        auto i = currentInstances.takeFirst();
        if (i != 0) {
            delete i;
        }
    }

    // TODO(rory): Currently we waste a stash tab fetch when we index, this could be made smarter
    if (foundIndex != -1 && foundIndex == instance->tab->tabIndex) {
        if (!error) {
            QJsonArray items = doc.object().value("items").toArray();
            ItemList itemObjects;
            for (QJsonValue itemVal : items) {
                QJsonObject item = itemVal.toObject();
                itemObjects.append(new Item(item));
            }
            instance->location->setItems(itemObjects);
            instance->location->setState(ItemLocation::Loaded);
        }

        // Mark error
        instance->error = error;

        emit onStashTabUpdateProgress(instance->tab->league, instance->tab->tabId, instance->throttled);

        _fetchingTabInstances.remove(key);

        saveStash(instance);
        emit onStashTabUpdate(instance->tab->league, instance->tab->tabId);
    }
    else {
        // Oh no! Failed to find the specified tab
        bool andThatsFine = (instance->tab->tabId.startsWith("index_"));
        if (!andThatsFine) qDebug() << "Failed to find " << instance->tab->tabId << " at " << instance->tab->tabIndex;
        if (foundIndex == -1) {
            if (!andThatsFine) qDebug() << "\tThe specified tab id does not exist.";
            _fetchingTabInstances.remove(key);
            _currentTabInstances.remove(key);

            delete instance;
        }
        else {
            qDebug() << "\tInstead it was found at: " << foundIndex;
            instance->tab->tabIndex = foundIndex;
            queueStashTab(instance);
        }
    }
}

void ItemManager::onCharacterItemsResult(QString character, QByteArray json, QVariant data) {
    Q_UNUSED(data)
    const QString key = "CHARACTER_" + character;
    ItemManagerInstance* instance = _fetchingCharacterInstances.value(key, nullptr);
    if (!instance) {
        qDebug() << "Failed to fetch instance for " << character;
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if (doc.isEmpty() || !doc.isObject() || doc.object().contains("error")) {
        qDebug() << "Failed to retrieve character (throttled?)";
        // TODO(rory): Handle throttling
        // {"error":{"message":"You are requesting your stash too frequently. Please try again later."}}
        qDebug() << qPrintable(json);
        _fetchingCharacterInstances.remove(key);
        return;
    }
    ItemManager::updateFetchable(instance);

    QJsonObject characterData = doc.object().value("character").toObject();
    characterData.insert("class", instance->character->classType);
    characterData.insert("level", instance->character->level);

    doc.object().insert("character", characterData);
    instance->location = new CharacterItemLocation(doc.object());

    _fetchingCharacterInstances.remove(key);

    ItemManagerInstance* in = _currentCharacterInstances.take(key);
    if (in) {
        delete in->character;
        delete in;
    }
    _currentCharacterInstances[key] = instance;

    emit onCharacterUpdate(instance->character->name);
}
