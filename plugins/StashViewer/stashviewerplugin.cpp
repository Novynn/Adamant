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
    _viewer = new StashViewer(Core()->interface()->window(), league);
    _characterViewer = new CharacterViewer(Core()->interface()->window());
    {
        QIcon icon(":/icons/dark/stack.png");
        Core()->interface()->window()->registerPage(icon, "Browse Stash", "View your stash tabs and items.", _viewer);
    }
    {
        QIcon icon(":/icons/dark/user.png");
        Core()->interface()->window()->registerPage(icon, "Characters", "View your characters.", _characterViewer);
    }
    connect(Core()->getItemManager(), &ItemManager::onStashTabUpdateAvailable, [this] (QString league) {
        // We only care about our current league
        if (league != Settings()->value("league").toString()) return;

        QList<StashItemLocation*> tabs = Core()->getItemManager()->GetStashTabs(league);
        _viewer->SetTabs(tabs);
    });

    connect(Core()->session(), &Session::Request::leaguesList, _viewer, &StashViewer::OnLeaguesList);
    connect(Core()->session(), &Session::Request::accountStashTabsJson, [this] (QString league, QJsonDocument doc, QVariant) {
        QStringList tabsList;
        for (QJsonValue tabVal : doc.object().value("tabs").toArray()) {
            QJsonObject tab = tabVal.toObject();
            tabsList << tab.value("n").toString();
        }
        if (!tabsList.isEmpty())
            _viewer->OnTabsList(league, tabsList);
    });

    connect(Core()->session(), &Session::Request::accountCharactersJson, [this] (QJsonDocument doc, QVariant) {
        QStringList names;
        for (QJsonValue charVal : doc.array()) {
            QJsonObject character = charVal.toObject();
            names << character.value("name").toString();
        }
        _characterViewer->setCharacters(names);
    });

    connect(_viewer, &StashViewer::RequestLeaguesList, Core()->session(), &Session::Request::fetchLeagues);
    connect(_viewer, &StashViewer::RequestStashTabList, [this](QString league) {
        const QString accountName = Core()->session()->accountName();
        Core()->session()->fetchAccountStashTabs(accountName, league);
    });

    connect(_viewer, &StashViewer::LeagueDetailsChanged, [this](QString league, QString filter) {
        Settings()->setValue("league", league);

        Settings()->beginGroup(league);
        Settings()->setValue("filter", filter);
        Settings()->endGroup();

        // Perform a fetch
        Core()->getItemManager()->fetchStashTabs(league, filter);
    });

    if (league.isEmpty()) {
        _viewer->ShowLeagueSelectionDialog();
    }
    else {
        Settings()->beginGroup(league);
        const QString filter = Settings()->value("filter").toString();
        Settings()->endGroup();

        // Perform a fetch
        Core()->getItemManager()->fetchStashTabs(league, filter);
    }

    Core()->session()->fetchAccountCharacters(Core()->session()->accountName());
}
