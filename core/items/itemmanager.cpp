#include "itemmanager.h"
#include "core.h"
#include "stashitemlocation.h"
#include "item.h"
#include <session/psession.h>

ItemManager::ItemManager(CoreService *parent)
    : QObject(parent)
    , _core(parent)
{
    connect(_core->Session(), &PSession::AccountStashTabs, this, &ItemManager::OnStashTabResult);
}

void ItemManager::FetchStashTabs(const QString &league, const QString &filter) {
    if (_fetchingInstances.contains(league)) {
        qDebug() << "Already fetching from " << league;
        return;
    }
    const QString accountName = _core->Session()->AccountName();
    ItemManagerInstance* instance = new ItemManagerInstance;
    instance->league = league;
    instance->filter = QRegularExpression(filter, QRegularExpression::CaseInsensitiveOption);
    instance->firstTabReceived = false;
    instance->receivedTabs = 0;
    instance->totalTabs = 0;

    if (accountName.isEmpty()) {
        qDebug() << "Account Name is empty!!!";
        return;
    }

    _fetchingInstances.insert(league, instance);

    _core->Session()->FetchAccountStashTabs(accountName, league, 0, true);
    emit OnStashTabUpdateBegin(league);
}

void ItemManager::OnStashTabResult(QString league, QByteArray json, QVariant data) {
    ItemManagerInstance* instance = _fetchingInstances.value(league, nullptr);
    if (!instance) return;
    QJsonDocument doc = QJsonDocument::fromJson(json);
    if (doc.isEmpty() || !doc.isObject()) {
        // TODO(rory): Log error
        qDebug() << "Failed to retrieve stash tabs (throttled?)";
        // TODO(rory): Handle throttling
        // {"error":{"message":"You are requesting your stash too frequently. Please try again later."}}
        qDebug() << qPrintable(json);
        return;
    }
    if (doc.object().contains("tabs") && !instance->firstTabReceived) {
        // This means it was a initial request, future requests for tab items should not have "tabs" set.
        // At least unless GGG changes their API.
        instance->firstTabReceived = true;

        QList<int> chosenIndicies;

        QJsonObject root = doc.object();
        QJsonArray tabs = root.value("tabs").toArray();
        for (QJsonValue tabVal : tabs) {
            QJsonObject tab = tabVal.toObject();
            // Check hidden flag
            if (tab.value("hidden").toBool()) continue;

            // Check filters
            QString name = tab.value("n").toString();
            if (!instance->filter.pattern().isEmpty() &&
                instance->filter.isValid() &&
                name.contains(instance->filter)) {
                continue;
            }

            int i = tab.value("i").toInt();
            chosenIndicies.append(i);

            ItemManagerInstanceTab* wrapper = new ItemManagerInstanceTab;
            StashItemLocation* location = new StashItemLocation(tab);
            wrapper->tabIndex = i;
            wrapper->location = location;
            wrapper->instance = instance;
            instance->tabs.insert(i, wrapper);
            const QString accountName = _core->Session()->AccountName();

            _core->Session()->FetchAccountStashTabs(accountName, instance->league, i, false, i);
        }

        instance->totalTabs = chosenIndicies.count();

        emit OnStashTabUpdateProgress(instance->league, instance->receivedTabs, instance->totalTabs);

        if (instance->totalTabs == 0) {
            // TODO(rory): Notify the user that no tabs were returned...
            emit OnStashTabUpdateAvailable(instance->league);
        }
    }
    else {
        int tabIndex = data.toInt();
        ItemManagerInstanceTab* wrapper = instance->tabs.value(tabIndex, nullptr);
        if (wrapper != nullptr) {
            // Add items data to some sort of container
            QJsonArray items = doc.object().value("items").toArray();
            ItemList itemObjects;
            for (QJsonValue itemVal : items) {
                QJsonObject item = itemVal.toObject();
                itemObjects.append(new Item(item));
            }
            wrapper->location->AddItems(itemObjects);

            instance->receivedTabs++;
            emit OnStashTabUpdateProgress(instance->league, instance->receivedTabs, instance->totalTabs);

            if (instance->receivedTabs == instance->totalTabs) {
                // We're done!
                _fetchingInstances.remove(instance->league);
                _currentInstances[league] = instance;
                emit OnStashTabUpdateAvailable(instance->league);
            }
        }
    }
}
