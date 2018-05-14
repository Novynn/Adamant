#ifndef STASHVIEWER_H
#define STASHVIEWER_H

#include "stashviewer_global.h"
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

class STASHVIEWER_EXTERN StashViewer : public QWidget
{
    Q_OBJECT

public:
    explicit StashViewer(QWidget *parent = 0, QString league = QString());
    ~StashViewer();
    QWidget* headerBar();

    Q_INVOKABLE void SetTabs(const QString& league, QList<StashItemLocation *> tabs);

    QString getCurrentLeague() const;
    QStringList getSelectedItems() const;
    QStringList getSelectedTabs() const;

    void AddTab(const QString& league, const StashItemLocation* tab);
    void LoadTabItem(const QString& tabId);
    void LoadTabItem(StashViewData* data);
    void LoadTab(StashViewData* data);
    void LoadTab(const QString& league, const ItemLocation* tab);
    void UpdateTab(const QString& league, const ItemLocation* tab, bool throttled);
protected:
    void timerEvent(QTimerEvent *);
public slots:
    void search(const QString& query = QString());
    void OnImage(const QString &path, QImage image);
    void OnLeaguesList(QStringList list);
    void OnViewportChanged();
private slots:
    void on_stashListWidget_itemSelectionChanged();
    void on_searchEdit_returnPressed();
    void on_searchEdit_textChanged(const QString &text);
    void on_leagueBox_currentIndexChanged(const QString &text);
    void on_updateButton_clicked();
    void on_autoUpdate_toggled(bool checked);
    void on_autoUpdateInterval_valueChanged(int val);

signals:
    void saveStash(const QString &league, const QString &id);
    void RequestStashTabList(const QString &league);
    void RequestStashTab(const QString &league, const QString &id);
    void LeagueDetailsChanged(const QString &league);
private:
    Ui::StashViewer *ui;

    StashScene* _scene;
    ImageCache* _imageCache;

    QHash<QString, StashViewData*> _tabs;

    QString _currentLeague;
    QStringList _leaguesList;

    GraphicItemFactory* _factory;
    QThread* _factoryThread;

    int _autoUpdateTimer;
};
Q_DECLARE_METATYPE(StashViewer*)

#endif // STASHVIEWER_H
