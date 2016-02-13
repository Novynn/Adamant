#include "itemmanager.h"
#include "core.h"
#include "stashitemlocation.h"
#include "item.h"
#include "characteritemlocation.h"
#include <QTimer>
#include <session/sessionrequest.h>

QQueue<Fetchable*> ItemManager::_fetchQueue;
QMap<Fetchable*, QDateTime> ItemManager::_fetchHistory;
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
    if (_fetchingInstances.contains(key)) {
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

    instance->character = new ItemManagerInstance::Character;
    instance->character->name = characterName;
    instance->character->location = nullptr;
    instance->character->classType = classType;
    instance->character->level = level;

    _fetchingInstances.insert(key, instance);

    queueCharacter(instance);
    beginFetch();
}

void ItemManager::fetchStashTabs(const QString &league, const QString &filter) {
    if (_fetchingInstances.contains(league)) {
        qDebug() << "Already fetching from " << league;
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

    instance->stash = new ItemManagerInstance::Stash;
    instance->stash->league = league;
    instance->stash->filter = QRegularExpression(filter, QRegularExpression::CaseInsensitiveOption);
    instance->stash->firstTabReceived = false;
    instance->stash->receivedTabs = 0;
    instance->stash->totalTabs = 0;

    _fetchingInstances.insert(league, instance);

    emit onStashTabUpdateBegin(league);

    queueFirstStashTab(instance);
    beginFetch();
}

void ItemManager::queueCharacter(ItemManagerInstance* instance) {
    ItemManager::_fetchQueue.enqueue(new Fetchable {Fetchable::Character, (void*)instance});
}

void ItemManager::queueFirstStashTab(ItemManagerInstance* instance) {
    ItemManager::_fetchQueue.enqueue(new Fetchable {Fetchable::Instance, (void*)instance});
}

void ItemManager::queueStashTab(ItemManagerInstanceTab* tab) {
    ItemManager::_fetchQueue.enqueue(new Fetchable {Fetchable::Tab, (void*)tab});
}

void ItemManager::fetchFirstStashTab(ItemManagerInstance *instance) {
    _core->session()->fetchAccountStashTabs(instance->accountName, instance->stash->league, 0, true);
}

void ItemManager::fetchCharacter(ItemManagerInstance* instance) {
    _core->session()->fetchAccountCharacterItems(instance->accountName, instance->character->name);
}

void ItemManager::fetchStashTab(ItemManagerInstanceTab* tab) {
    QString accountName = tab->instance->accountName;
    QString league = tab->instance->stash->league;
    _core->session()->fetchAccountStashTabs(accountName, league, tab->tabIndex, false, tab->tabIndex);
}

void ItemManager::beginFetch() {
    if (ItemManager::_fetchWaiting) return;

    // Let's get to work!
    pruneHistory();
    // Only fetch tabs if we haven't gotten 45 in the last minute (this is how it works on GGG's end).
    if (_fetchHistory.count() < ItemManager::RequestsPerPeriod) {
        while (!ItemManager::_fetchQueue.isEmpty()) {
            Fetchable* fetchable = ItemManager::_fetchQueue.dequeue();

            ItemManagerInstance* instance = nullptr;
            ItemManager* manager = nullptr;
            ItemManager::_fetchHistory.insert(fetchable, QDateTime());
            switch (fetchable->type) {
                case Fetchable::Instance: {
                    instance = fetchable->instance;
                    manager = instance->manager;
                    manager->fetchFirstStashTab(instance);
                } break;
                case Fetchable::Character: {
                    instance = fetchable->instance;
                    manager = instance->manager;
                    manager->fetchCharacter(instance);
                } break;
                case Fetchable::Tab: {
                    auto tab = fetchable->tab;
                    instance = tab->instance;
                    manager = instance->manager;
                    manager->fetchStashTab(tab);
                } break;
            }

            if (ItemManager::_fetchHistory.count() >= ItemManager::RequestsPerPeriod) {
                qInfo() << "Stash tab fetching has been throttled.";
                for (auto i : manager->_fetchingInstances.values()) {
                    emit manager->onStashTabUpdateProgress(i->stash->league, i->stash->receivedTabs, i->stash->totalTabs, i->throttled = true);
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
    QMap<Fetchable*, QDateTime> newHistory;
    while (!ItemManager::_fetchHistory.isEmpty()) {
        auto fetchable = ItemManager::_fetchHistory.firstKey();
        QDateTime date = ItemManager::_fetchHistory.take(fetchable);
        if (date.isNull() || date.msecsTo(now) < ItemManager::RequestPeriodMSecs) {
            newHistory.insert(fetchable, date);
        }
        else {
            delete fetchable;
        }
    }
    int removed = start - newHistory.size();
    ItemManager::_fetchHistory = newHistory;
    if (removed) {
        // qInfo() << qPrintable("History pruning complete: " + QString::number(removed) + (removed == 1 ? " entry" : " entries") + " deleted.");
    }
}

void ItemManager::updateFetchable(Fetchable* fetchable) {
    ItemManager::_fetchHistory.remove(fetchable);
    ItemManager::_fetchHistory.insert(fetchable, QDateTime::currentDateTime());
}

void ItemManager::updateFetchable(ItemManagerInstance* instance, int type) {
    for (Fetchable* fetchable : ItemManager::_fetchHistory.keys()) {
        if (static_cast<int>(fetchable->type) == type && fetchable->instance == instance) {
            updateFetchable(fetchable);
            break;
        }
    }
}

void ItemManager::updateFetchable(ItemManagerInstanceTab* tab) {
    for (Fetchable* fetchable : ItemManager::_fetchHistory.keys()) {
        if (fetchable->type == Fetchable::Tab && fetchable->tab == tab) {
            updateFetchable(fetchable);
            break;
        }
    }
}

void ItemManager::onStashTabResult(QString league, QByteArray json, QVariant data) {
    ItemManagerInstance* instance = _fetchingInstances.value(league, nullptr);
    if (!instance) {
        //qDebug() << "Failed to fetch instance for " << league;
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(json);
    bool error = false;
    if (doc.isEmpty() || !doc.isObject() || doc.object().contains("error")) {
        // TODO(rory): Log error
        qDebug() << "Failed to retrieve stash tabs (throttled?)";
        // TODO(rory): Handle throttling
        // {"error":{"message":"You are requesting your stash too frequently. Please try again later."}}
        qDebug() << qPrintable(json);

        if (!instance->stash->firstTabReceived) {
            // We failed to get the first tab (uh oh).
            qWarning() << qPrintable("Failed to get the first tab.");
            _fetchingInstances.remove(instance->stash->league);
            emit onStashTabUpdateProgress(instance->stash->league, 0, 0, false);
            emit onStashTabUpdateAvailable(instance->stash->league);
            return;
        }

        error = true;
    }
    if (!instance->stash->firstTabReceived) {
        if (!doc.object().contains("tabs")) {
            qDebug() << "??? result did not contain tabs...";
        }
        // This means it was a initial request, future requests for tab items should not have "tabs" set.
        // At least unless GGG changes their API.
        instance->stash->firstTabReceived = true;
        ItemManager::updateFetchable(instance);

        QList<int> chosenIndicies;

        QJsonObject root = doc.object();
        QJsonArray tabs = root.value("tabs").toArray();
        for (QJsonValue tabVal : tabs) {
            QJsonObject tab = tabVal.toObject();
            // Check hidden flag
            if (tab.value("hidden").toBool()) continue;

            // Check filters
            QString name = tab.value("n").toString();
            if (!instance->stash->filter.pattern().isEmpty() &&
                instance->stash->filter.isValid() &&
                name.contains(instance->stash->filter)) {
                continue;
            }

            int i = tab.value("i").toInt();
            chosenIndicies.append(i);

            ItemManagerInstanceTab* wrapper = new ItemManagerInstanceTab;
            StashItemLocation* location = new StashItemLocation(tab);
            wrapper->tabIndex = i;
            wrapper->location = location;
            wrapper->instance = instance;
            wrapper->error = false;
            instance->stash->tabs.insert(i, wrapper);

            queueStashTab(wrapper);
        }
        beginFetch();

        instance->stash->totalTabs = chosenIndicies.count();

        emit onStashTabUpdateProgress(instance->stash->league, instance->stash->receivedTabs, instance->stash->totalTabs, instance->throttled);

        if (instance->stash->totalTabs == 0) {
            // TODO(rory): Notify the user that no tabs were returned...
            emit onStashTabUpdateAvailable(instance->stash->league);
        }
    }
    else {
        int tabIndex = data.toInt();
        ItemManagerInstanceTab* wrapper = instance->stash->tabs.value(tabIndex, nullptr);
        if (wrapper != nullptr) {
            // Update our history
            ItemManager::updateFetchable(wrapper);

            // Add items data to some sort of container
            // Potientially this is empty, as an error will come through here
            QJsonArray items = doc.object().value("items").toArray();
            ItemList itemObjects;
            for (QJsonValue itemVal : items) {
                QJsonObject item = itemVal.toObject();
                itemObjects.append(new Item(item));
            }
            wrapper->location->addItems(itemObjects);

            // Mark error
            wrapper->error = error;

            instance->stash->receivedTabs++;
            emit onStashTabUpdateProgress(instance->stash->league, instance->stash->receivedTabs, instance->stash->totalTabs, instance->throttled);

            if (instance->stash->receivedTabs == instance->stash->totalTabs) {
                // We're done!
                _fetchingInstances.remove(instance->stash->league);

                ItemManagerInstance* in = _currentInstances.take(league);
                if (in) {
                    for (ItemManagerInstanceTab* tab : in->stash->tabs) {
                        delete tab->location;
                        delete tab;
                    }
                    in->stash->tabs.clear();
                    delete in->stash;
                    delete in;
                }
                _currentInstances[league] = instance;
                emit onStashTabUpdateAvailable(instance->stash->league);
            }
        }
    }
}

void ItemManager::onCharacterItemsResult(QString character, QByteArray json, QVariant data) {
    Q_UNUSED(data)
    const QString key = "CHARACTER_" + character;
    ItemManagerInstance* instance = _fetchingInstances.value(key, nullptr);
    if (!instance) {
//        qDebug() << "Failed to fetch instance for " << character;
        return;
    }
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if (doc.isEmpty() || !doc.isObject() || doc.object().contains("error")) {
        // TODO(rory): Log error
        qDebug() << "Failed to retrieve character (throttled?)";
        // TODO(rory): Handle throttling
        // {"error":{"message":"You are requesting your stash too frequently. Please try again later."}}
        qDebug() << qPrintable(json);
        _fetchingInstances.remove(key);
        return;
    }
    ItemManager::updateFetchable(instance, Fetchable::Character);

    QJsonObject characterData = doc.object().value("character").toObject();
    characterData.insert("class", instance->character->classType);
    characterData.insert("level", instance->character->level);

    doc.object().insert("character", characterData);

    auto location = new CharacterItemLocation(doc.object());
    instance->character->location = location;

    _fetchingInstances.remove(key);

    ItemManagerInstance* in = _currentInstances.take(key);
    if (in) {
        delete in->character;
        delete in;
    }
    _currentInstances[key] = instance;

    emit onCharacterUpdateAvailable(instance->character->name);
}
