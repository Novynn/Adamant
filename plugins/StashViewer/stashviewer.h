#ifndef STASHVIEWER_H
#define STASHVIEWER_H

#include <QThread>
#include <QWidget>
#include "graphicitemfactory.h"
#include "stashscene.h"
#include <QGraphicsRectItem>
#include <QJsonDocument>
#include <session/imagecache.h>

namespace Ui {
class StashViewer;
}

class QTabBar;
class StashItemLocation;
class GraphicItem;
class LeagueDialog;
class QListWidgetItem;

class StashViewer : public QWidget
{
    Q_OBJECT

public:
    explicit StashViewer(QWidget *parent = 0, QString league = QString());
    ~StashViewer();

    Q_INVOKABLE void SetTabs(QList<StashItemLocation *> tabs);

public slots:
    void OnImage(const QString &path, QImage image);
    void OnLeaguesList(QStringList list);
    void OnTabsList(QString league, QStringList list);

    void ShowLeagueSelectionDialog();
private slots:
    void on_listWidget_itemSelectionChanged();
    void on_lineEdit_returnPressed();

    void on_leagueButton_clicked();
    void on_lineEdit_textChanged(const QString &text);

signals:
    void RequestLeaguesList();
    void RequestStashTabList(QString league);
    void LeagueDetailsChanged(QString league, QString filter);
private:
    Ui::StashViewer *ui;

    QTabBar* _bar;
    StashScene* _scene;
    ImageCache* _imageCache;

    QHash<QListWidgetItem* ,QGraphicsPixmapItem*> _tabGrids;
    QHash<QString, StashItemLocation*> _tabs;

    LeagueDialog* _leagueDialog;
    QString _currentLeague;
    GraphicItemFactory* _factory;
    QThread* _factoryThread;
};
Q_DECLARE_METATYPE(StashViewer*)

#endif // STASHVIEWER_H
