#include "graphicitem.h"
#include "stashviewer.h"
#include "ui_stashviewer.h"

#include <QScrollBar>
#include <QGraphicsPixmapItem>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

#include <items/item.h>
#include <items/stashitemlocation.h>

#include <dialogs/leaguedialog.h>

StashViewer::StashViewer(QWidget *parent, QString league)
    : QWidget(parent)
    , ui(new Ui::StashViewer)
    , _scene(new StashScene(this))
    , _imageCache(new ImageCache)
    , _leagueDialog(new LeagueDialog(this))
    , _currentLeague(league)
    , _factory(new GraphicItemFactory(_imageCache))
    , _factoryThread(new QThread(this))
{
    ui->setupUi(this);

    connect(_imageCache, &ImageCache::onImage, this, &StashViewer::OnImage);

    _factory->moveToThread(_factoryThread);
    connect(_factory, &GraphicItemFactory::OnItemsReady, this, [this] (QList<GraphicItem*> items, QSet<QString> images, QVariant data) {
        QGraphicsItem* parent = data.value<QGraphicsItem*>();
        if (parent) {
            for (auto item : items) {
                item->setParentItem(parent);
            }
            parent->setOpacity(1.0);
        }

        for (const QString &image : images) {
            _imageCache->fetchImage(image);
        }
    });
    connect(_factory, &GraphicItemFactory::destroyed, _factoryThread, &QThread::deleteLater);
    _factoryThread->start();

    connect(_leagueDialog, &LeagueDialog::RequestStashTabList, [this] (QString league) {
        emit RequestStashTabList(league);
    });

    connect(ui->graphicsView->verticalScrollBar(), &QScrollBar::valueChanged, this, &StashViewer::OnViewportChanged);
    connect(ui->graphicsView->verticalScrollBar(), &QScrollBar::rangeChanged, this, &StashViewer::OnViewportChanged);

    _scene->setBackgroundBrush(QBrush(Qt::transparent));
    ui->graphicsView->setBackgroundBrush(QBrush(Qt::transparent));
    ui->graphicsView->setScene(_scene);
}

void StashViewer::OnViewportChanged() {
    if (_tabGrids.isEmpty()) return;

    // Setup a nice rect in the center of the viewport to show which grids are in "focus".
    QRectF viewport = ui->graphicsView->mapToScene(ui->graphicsView->viewport()->rect()).boundingRect();
    QPointF center = viewport.center();
    QRectF gridRect = _tabGrids.values().first()->boundingRect(); // Steal the rect of one of the grids (ew, dirty)
    gridRect.translate(-gridRect.height() / 2 + center.x(), -gridRect.width() / 2 + center.y());

    QList<QGraphicsItem*> items = _scene->items(gridRect, Qt::IntersectsItemBoundingRect, Qt::AscendingOrder, ui->graphicsView->transform());
    QList<QGraphicsItem*> filtered;
    for (auto item : items) {
        if (item->topLevelItem() == item) filtered << item;
    }

    for (auto listItem : _tabGrids.keys()) {
        auto grid = _tabGrids.value(listItem);

        if (filtered.contains(grid)) {
            listItem->setTextAlignment(Qt::AlignRight);
            ui->listWidget->scrollToItem(listItem); // Ensures the item is visible
        }
        else {
            listItem->setTextAlignment(Qt::AlignLeft);
        }
    }
}

void StashViewer::OnImage(const QString &path, QImage image) {
    // TODO(rory): Optimize!
    for (QGraphicsPixmapItem* p : _tabGrids) {
        for (QGraphicsItem* i : p->childItems()) {
            GraphicItem* item = dynamic_cast<GraphicItem*>(i);
            if (item && item->IsWaitingForImage(path)) {
                item->SetImage(image);
            }
        }
    }
}

QStringList LeaguesList;

void StashViewer::OnLeaguesList(QStringList list) {
    _leagueDialog->SetLeagues(list);
    LeaguesList = list;
}

void StashViewer::OnTabsList(QString league, QStringList list) {
    _leagueDialog->SetLeagueTabs(league, list);
}

void StashViewer::ShowLeagueSelectionDialog() {
    if (LeaguesList.isEmpty())
        emit RequestLeaguesList();
    else
        _leagueDialog->SetLeagues(LeaguesList);
    if (_leagueDialog->exec() == QDialog::Accepted) {
        _currentLeague = _leagueDialog->GetChosenLeague();
        emit LeagueDetailsChanged(_currentLeague, _leagueDialog->GetFilter());
    }
    _leagueDialog->Clear();
}

StashViewer::~StashViewer() {
    delete ui;
}

void StashViewer::SetTabs(QList<StashItemLocation*> tabs) {
    // Clear Tabs
    ui->listWidget->clear();
    _tabs.clear();
    // Clear items
    _scene->clear();
    _tabGrids.clear();

    // Load new tabs
    for (StashItemLocation* tab : tabs) {
        QString header = dynamic_cast<ItemLocation*>(tab)->header();
        QListWidgetItem* item = new QListWidgetItem(header, ui->listWidget);
        QColor background = tab->tabColor();
        QColor foreground;
        item->setBackgroundColor(background);
        if (background.lightnessF() > 0.5)
            foreground = QColor(Qt::black);
        else
            foreground = QColor(Qt::white);
        item->setForeground(foreground);
        ui->listWidget->addItem(item);

        _tabs.insert(header, tab);

        // Setup Grid Items
        static QPixmap grid(":/images/StashPanelGrid.png");
        QGraphicsPixmapItem* gridItem = _scene->addPixmap(grid);
        gridItem->setVisible(false);
        _tabGrids.insert(item, gridItem);

        // Invoke item loading, right here right now
        gridItem->setOpacity(0.1);
        QMetaObject::invokeMethod(_factory,
                                  "SubmitLocation",
                                  Qt::QueuedConnection,
                                  Q_ARG(const ItemLocation*, tab),
                                  Q_ARG(QVariant, QVariant::fromValue<QGraphicsItem*>(gridItem)));
    }
}

void StashViewer::on_listWidget_itemSelectionChanged() {
    QList<QListWidgetItem*> items = ui->listWidget->selectedItems();

    // Sort into the same order that the QListWidget contains them as
    std::sort(items.begin(), items.end(), [this](const QListWidgetItem* a, const QListWidgetItem* b) -> bool {
        return ui->listWidget->row(a) < ui->listWidget->row(b);
    });

    static QPixmap grid(":/images/StashPanelGrid.png");
    static QSize stashPanelSize = grid.size();

    // Calculate Scene Bounds
    int finalHeight = stashPanelSize.height();
    static const int Padding = 16;
    if (items.length() > 1) {
        finalHeight = (stashPanelSize.height() + Padding) * items.length();
    }
    _scene->setSceneRect(0, 0, stashPanelSize.width(), finalHeight);

    int index = 0;
    QStringList tabLabels;

    for (QListWidgetItem* item : _tabGrids.keys()) {
        QGraphicsPixmapItem* gridItem = _tabGrids.value(item);
        if (!items.contains(item)) gridItem->hide();
        else gridItem->show();
    }

    for (QListWidgetItem* item : items) {
        QGraphicsPixmapItem* gridItem = _tabGrids.value(item);

        // Save label
        tabLabels.append(item->text());

        // Move into the correct position
        int height = (stashPanelSize.height() + Padding) * index;
        gridItem->setPos(0, height);
        index++;
    }

    emit ui->lineEdit->returnPressed();
}

void StashViewer::on_lineEdit_returnPressed() {
    const QString &text = ui->lineEdit->text();
    for (int i = 0; i < ui->listWidget->count(); i++) {
        QListWidgetItem* item = ui->listWidget->item(i);
        QGraphicsPixmapItem* gridItem = _tabGrids.value(item);

        bool showGrid = false;
        for (QGraphicsItem* gi : gridItem->childItems()) {
            GraphicItem* gitem = dynamic_cast<GraphicItem*>(gi);
            if (gitem) {
                if (text.isEmpty() || !gitem->IsFilteredBy(text)) {
                    gitem->setEnabled(true);
                    showGrid = true;
                }
                else {
                    gitem->setEnabled(false);
                }
            }
        }
        if (text.isEmpty()) {
            item->setHidden(false);
        }
        else {
            item->setSelected(showGrid);
            item->setHidden(!showGrid);
        }
    }
    OnViewportChanged();

    // TODO(rory): This should not be needed here
    // _scene->invalidate();
}

void StashViewer::on_leagueButton_clicked() {
    ShowLeagueSelectionDialog();
}

void StashViewer::on_lineEdit_textChanged(const QString &text) {
    if (text.isEmpty()) {
        emit ui->lineEdit->returnPressed();
    }
}
