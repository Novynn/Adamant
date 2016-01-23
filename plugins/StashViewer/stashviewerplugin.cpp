#include "stashviewerplugin.h"
#include "stashviewer.h"
#include <items/itemmanager.h>
#include <items/stashitemlocation.h>

#include <QDebug>

StashViewerPlugin::StashViewerPlugin()
    : _viewer(nullptr) {
}

void StashViewerPlugin::OnLoad() {
    const QString league = Settings()->value("league").toString();
    _viewer = new StashViewer(Core()->Interface()->Window(), league);
    {
        QIcon icon(":/icons/dark/stack.png");
        Core()->Interface()->Window()->RegisterPage(icon, "Browse Stash", "View your stash tabs and items.", _viewer);
    }
    {
        QIcon icon(":/icons/dark/user.png");
        Core()->Interface()->Window()->RegisterPage(icon, "Characters", "View your characters.", new QWidget());
    }
    connect(Core()->GetItemManager(), &ItemManager::OnStashTabUpdateAvailable, [this] (QString league) {
        // We only care about our current league
        if (league != Settings()->value("league").toString()) return;

        QList<StashItemLocation*> tabs = Core()->GetItemManager()->GetStashTabs(league);
        _viewer->SetTabs(tabs);
    });

    connect(Core()->Session(), &PSession::LeaguesList, _viewer, &StashViewer::OnLeaguesList);
    connect(Core()->Session(), &PSession::AccountStashTabsJson, [this] (QString league, QJsonDocument doc, QVariant) {
        QStringList tabsList;
        for (QJsonValue tabVal : doc.object().value("tabs").toArray()) {
            QJsonObject tab = tabVal.toObject();
            tabsList << tab.value("n").toString();
        }
        if (!tabsList.isEmpty())
            _viewer->OnTabsList(league, tabsList);
    });

    connect(_viewer, &StashViewer::RequestLeaguesList, Core()->Session(), &PSession::FetchLeagues);
    connect(_viewer, &StashViewer::RequestStashTabList, [this](QString league) {
        const QString accountName = Core()->Session()->AccountName();
        Core()->Session()->FetchAccountStashTabs(accountName, league);
    });

    connect(_viewer, &StashViewer::LeagueDetailsChanged, [this](QString league, QString filter) {
        Settings()->setValue("league", league);

        Settings()->beginGroup(league);
        Settings()->setValue("filter", filter);
        Settings()->endGroup();

        // Perform a fetch
        Core()->GetItemManager()->FetchStashTabs(league, filter);
    });

    if (league.isEmpty()) {
        _viewer->ShowLeagueSelectionDialog();
    }
    else {
        Settings()->beginGroup(league);
        const QString filter = Settings()->value("filter").toString();
        Settings()->endGroup();

        // Perform a fetch
        Core()->GetItemManager()->FetchStashTabs(league, filter);
    }
}
