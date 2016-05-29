#include "stashviewerplugin.h"
#include "stashviewer.h"
#include <items/itemmanager.h>
#include <items/stashitemlocation.h>

#include <QDebug>

StashViewerPlugin::StashViewerPlugin()
    : _viewer(nullptr)
    , _characterViewer(nullptr) {
}

void StashViewerPlugin::OnLoad() {
    const QString league = Settings()->value("league").toString();
    _viewer = new StashViewer(Core()->getInterface()->window(), league);
    _characterViewer = new CharacterViewer(Core()->getInterface()->window());
    Core()->getInterface()->registerPluginPage(this, QIcon(":/icons/dark/stack.png"),
                                               "Browse Stash", "View your stash tabs and items.",
                                               _viewer);
    Core()->getInterface()->registerPluginPage(this, QIcon(":/icons/dark/user.png"),
                                               "Characters", "View your characters.",
                                               _characterViewer);

    connect(Core()->getItemManager(), &ItemManager::onStashTabIndexMapUpdate, [this] (QString league) {
        QList<StashItemLocation*> tabs = Core()->getItemManager()->getStashTabs(league);
        QStringList tabsList;
        for (auto loc : tabs) {
            tabsList << loc->header();
        }
        _viewer->SetTabs(league, tabs);
    });

    connect(Core()->getItemManager(), &ItemManager::onStashTabUpdate, [this] (QString league, QString id) {
        StashItemLocation* tab = Core()->getItemManager()->getStashTab(id);
        if (tab != nullptr)
            _viewer->LoadTab(league, tab);
    });

    connect(Core()->getItemManager(), &ItemManager::onStashTabUpdateProgress, [this] (QString league, QString tabId, bool throttled) {
        StashItemLocation* tab = Core()->getItemManager()->getStashTab(tabId);
        if (tab != nullptr)
            _viewer->UpdateTab(league, tab, throttled);
    });

    Core()->settings()->beginGroup("data");
    const QStringList leagues = Core()->settings()->value("leagues").toStringList();
    Core()->settings()->endGroup();
    _viewer->OnLeaguesList(leagues);
    _characterViewer->setLeagues(leagues);

    connect(Core()->session(), &Session::Request::accountCharactersJson, [this] (QJsonDocument doc, QVariant) {
        QList<Character> characters;
        for (QJsonValue charVal : doc.array()) {
            QJsonObject charObj = charVal.toObject();
            Character character = {
                                    charObj.value("name").toString(),
                                    charObj.value("league").toString(),
                                    charObj.value("class").toString(),
                                    charObj.value("level").toInt()
                                  };
            Core()->getItemManager()->fetchCharacterItems(character.name, character.classType, character.level);
            characters << character;
        }

        _characterViewer->setCharacters(characters);
    });

    connect(Core()->getItemManager(), &ItemManager::onCharacterUpdate, [this] (QString character) {
        auto location = Core()->getItemManager()->getCharacterItems(character);
        _characterViewer->setCharacterItems(character, location);
    });

    connect(_viewer, &StashViewer::RequestLeaguesList, Core()->session(), &Session::Request::fetchLeagues);
    connect(_viewer, &StashViewer::RequestStashTabList, [this](QString league) {
        Core()->getItemManager()->fetchStashTab(league, "");
    });
    connect(_viewer, &StashViewer::RequestStashTab, [this](QString league, QString id) {
        Core()->getItemManager()->fetchStashTab(league, id);
    });

    connect(_viewer, &StashViewer::LeagueDetailsChanged, [this](QString league) {
        Settings()->setValue("league", league);
    });

    if (!league.isEmpty()) {
        // Perform a fetch
        qDebug() << "Auto-fetching " << league;
        Core()->getItemManager()->fetchStashTab(league, "");
    }

    //Core()->getItemManager()->fe();
    Core()->session()->fetchAccountCharacters(Core()->session()->accountName());
}
