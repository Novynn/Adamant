#ifndef STASHVIEWER_H
#define STASHVIEWER_H

#include <QThread>
#include <QWidget>
#include "graphicitemfactory.h"
#include "stashscene.h"
#include <QGraphicsRectItem>
#include <QJsonDocument>
#include <session/imagecache.h>
#include <stash/stashviewdata.h>

namespace Ui {
class StashViewer;
}

class QTabBar;
class StashItemLocation;
class GraphicItem;
class QListWidgetItem;

class StashViewer : public QWidget
{
    Q_OBJECT

public:
    explicit StashViewer(QWidget *parent = 0, QString league = QString());
    ~StashViewer();
    QWidget* headerBar();

    Q_INVOKABLE void SetTabs(const QString& league, QList<StashItemLocation *> tabs);

    void AddTab(const QString& league, const ItemLocation* tab);
    void LoadTabItem(const QString& tabId);
    void LoadTabItem(StashViewData* data);
    void LoadTab(StashViewData* data);
    void LoadTab(const QString& league, const ItemLocation* tab);
    void UpdateTab(const QString& league, const ItemLocation* tab, bool throttled);
public slots:
    void OnImage(const QString &path, QImage image);
    void OnLeaguesList(QStringList list);
    void OnViewportChanged();
private slots:
    void on_stashListWidget_itemSelectionChanged();
    void on_searchEdit_returnPressed();
    void on_searchEdit_textChanged(const QString &text);
    void on_leagueBox_currentIndexChanged(const QString &text);
    void on_updateButton_clicked();

signals:
    void RequestLeaguesList();
    void RequestStashTabList(QString league);
    void RequestStashTab(QString league, QString id);
    void LeagueDetailsChanged(QString league);
private:
    Ui::StashViewer *ui;

    StashScene* _scene;
    ImageCache* _imageCache;

    QHash<QString, StashViewData*> _tabs;

    QString _currentLeague;
    QStringList _leaguesList;

    GraphicItemFactory* _factory;
    QThread* _factoryThread;

    QGraphicsProxyWidget* _optionsBarProxy;
};
Q_DECLARE_METATYPE(StashViewer*)

#endif // STASHVIEWER_H
