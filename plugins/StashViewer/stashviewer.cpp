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

StashViewer::StashViewer(QWidget *parent, QString league)
    : QWidget(parent)
    , ui(new Ui::StashViewer)
    , _scene(new StashScene(this))
    , _imageCache(new ImageCache)
    , _currentLeague(league)
    , _factory(new GraphicItemFactory(_imageCache))
    , _factoryThread(new QThread(this))
{
    ui->setupUi(this);
//    ui->viewWidget->layout()->removeWidget(ui->optionsBar);
//    ui->optionsBar->setParent(nullptr);


    connect(_imageCache, &ImageCache::onImage, this, &StashViewer::OnImage);

    _factory->moveToThread(_factoryThread);
    connect(_factory, &GraphicItemFactory::OnItemsReady, this, [this] (QList<GraphicItem*> items, QSet<QString> images, QVariant vData) {
        StashViewData* data = vData.value<StashViewData*>();
        Q_ASSERT(data);
        QGraphicsItem* parent = data->getGrid();
        if (parent) {
            for (auto item : items) {
                item->setParentItem(parent);
            }
            data->setLoaded();
        }

        for (const QString &image : images) {
            _imageCache->fetchImage(image);
        }
    });
    connect(_factory, &GraphicItemFactory::destroyed, _factoryThread, &QThread::deleteLater);
    _factoryThread->start();

    connect(ui->graphicsView->verticalScrollBar(), &QScrollBar::valueChanged, this, &StashViewer::OnViewportChanged);
    connect(ui->graphicsView->verticalScrollBar(), &QScrollBar::rangeChanged, this, &StashViewer::OnViewportChanged);

    _scene->setBackgroundBrush(QBrush(Qt::transparent));
    ui->graphicsView->setBackgroundBrush(QBrush(Qt::transparent));
    ui->graphicsView->setScene(_scene);

    _optionsBarProxy = nullptr; //_scene->addWidget(ui->optionsBar, Qt::Widget);
}

void StashViewer::OnViewportChanged() {
    if (_tabs.isEmpty()) return;

    // Setup a nice rect in the center of the viewport to show which grids are in "focus".
    QRectF viewport = ui->graphicsView->mapToScene(ui->graphicsView->viewport()->rect()).boundingRect();
    QPointF center = viewport.center();
    QRectF gridRect = _tabs.values().first()->getGrid()->boundingRect(); // Steal the rect of one of the grids (ew, dirty)
    gridRect.translate(-gridRect.height() / 2 + center.x(), -gridRect.width() / 2 + center.y());

    QList<QGraphicsItem*> items = _scene->items(gridRect, Qt::IntersectsItemBoundingRect, Qt::AscendingOrder, ui->graphicsView->transform());
    QList<QGraphicsItem*> filtered;
    for (auto item : items) {
        if (item->topLevelItem() == item) filtered << item;
    }

    for (auto data : _tabs.values()) {
        auto grid = data->getGrid();
        auto listItem = data->getItem();

        if (filtered.contains(grid)) {
            ui->stashListWidget->scrollToItem(listItem); // Ensures the item is visible
        }
        else {
            listItem->setTextAlignment(Qt::AlignLeft);
        }
    }
}

void StashViewer::OnImage(const QString &path, QImage image) {
    // TODO(rory): Optimize!
    for (auto data : _tabs) {
        QGraphicsPixmapItem* p = data->getGrid();
        for (QGraphicsItem* i : p->childItems()) {
            GraphicItem* item = dynamic_cast<GraphicItem*>(i);
            if (item && item->IsWaitingForImage(path)) {
                item->SetImage(image);
            }
        }
    }
}

void StashViewer::OnLeaguesList(QStringList list) {
    _leaguesList = list;
    ui->leagueBox->blockSignals(true);
    ui->leagueBox->clear();
    ui->leagueBox->addItems(list);
    ui->leagueBox->setCurrentText(_currentLeague);
    ui->leagueBox->blockSignals(false);
}

StashViewer::~StashViewer() {
    delete ui;
}

QWidget* StashViewer::headerBar() {
    return ui->headerBar;
}

void StashViewer::LoadTabItem(const QString &tabId) {
    auto data = _tabs.value(tabId, nullptr);
    LoadTabItem(data);
}

void StashViewer::LoadTabItem(StashViewData* data) {
    auto loc = dynamic_cast<const StashItemLocation*>(data->getLocation());
    QColor background = loc ? loc->tabColor() : QColor(Qt::red);
    data->getItem()->setBackgroundColor(background);
    QColor foreground;
    QString colorHint;
    if (background.lightnessF() > 0.5) {
        foreground = QColor(Qt::black);
        colorHint = "light";
    }
    else {
        foreground = QColor(Qt::white);
        colorHint = "dark";
    }
    data->getItem()->setForeground(foreground);
    data->getItem()->setToolTip("Last Updated: Never");
    if (data->getItem()->icon().isNull()) {
        data->getItem()->setIcon(QIcon(QString(":/icons/%1/question.png").arg(colorHint)));
    }
}

void StashViewer::SetTabs(const QString &league, QList<StashItemLocation*> tabs) {
    if (_currentLeague != league) return;
    QStringList ids = _tabs.keys();
    // Load new tabs, update old ones!
    for (StashItemLocation* tab : tabs) {
        auto data = _tabs.value(tab->hash(), nullptr);
        if (data != nullptr) {
            Q_ASSERT(data->getLocation() == tab);
            LoadTabItem(data);
//            if (tab->state() == ItemLocation::Loaded)
//                LoadTab(league, tab);
            ids.removeOne(tab->hash());
            continue;
        }

        AddTab(league, tab);
    }

    for (QString id : ids) {
        // To remove!
        StashViewData* data = _tabs.take(id);
        Q_ASSERT(data);

        ui->stashListWidget->removeItemWidget(data->getItem());
        _scene->removeItem(data->getGrid());

        delete data;
    }

    // Realign tabs to fit the new tab map
}

void StashViewer::AddTab(const QString &league, const ItemLocation* tab)
{
    static QPixmap grid(":/images/StashPanelGrid.png");
    const QString header = tab->header();


    QListWidgetItem* item = new QListWidgetItem(header, ui->stashListWidget);
    ui->stashListWidget->addItem(item);

    // Setup Grid Items
    QGraphicsPixmapItem* gridItem = _scene->addPixmap(grid);
    gridItem->setVisible(false);

    StashViewData* data = new StashViewData(tab, item, gridItem);
    _tabs.insert(tab->hash(), data);

    item->setData(StashViewData::ListItemDataIndex, QVariant::fromValue<StashViewData*>(data));

    LoadTabItem(data);

    if (!tab->state() == ItemLocation::Loaded) {
        LoadTab(league, tab);
    }
    else {
        data->setLoaded(false);
    }

}

void StashViewer::LoadTab(StashViewData* data)
{
    // Clear out old items
    QList<QGraphicsItem*> children = data->getGrid()->childItems();
    while (!children.isEmpty()) {
        auto item = children.takeFirst();
        if (!dynamic_cast<GraphicItem*>(item)) continue;
        item->setParentItem(nullptr);
        _scene->removeItem(item);
        delete item;
    }

    data->setLoaded(false);
//    item->setIcon(QIcon(QString(":/icons/%1/cloud-download.png").arg(iconType)));
    QMetaObject::invokeMethod(_factory,
                              "SubmitLocation",
                              Qt::QueuedConnection,
                              Q_ARG(const ItemLocation*, data->getLocation()),
                              Q_ARG(QVariant, QVariant::fromValue<StashViewData*>(data)));
}

void StashViewer::UpdateTab(const QString &league, const ItemLocation* tab, bool throttled) {
    Q_UNUSED(league);

    auto data = _tabs.value(tab->hash(), nullptr);
    if (data) {
        data->setLoaded(false, throttled);
    }
}

void StashViewer::LoadTab(const QString &league, const ItemLocation* tab) {
    if (_currentLeague != league) return;
    auto data = _tabs.value(tab->hash(), nullptr);
    Q_ASSERT(data != nullptr);
    LoadTab(data);
}

void StashViewer::on_stashListWidget_itemSelectionChanged() {
    QList<QListWidgetItem*> items = ui->stashListWidget->selectedItems();

    // Sort into the same order that the QListWidget contains them as
    std::sort(items.begin(), items.end(), [this](const QListWidgetItem* a, const QListWidgetItem* b) -> bool {
        return ui->stashListWidget->row(a) < ui->stashListWidget->row(b);
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

    for (auto data : _tabs.values()) {
        if (!items.contains(data->getItem())) data->getGrid()->hide();
        else data->getGrid()->show();
    }

    for (QListWidgetItem* item : items) {
        StashViewData* data = item->data(StashViewData::ListItemDataIndex).value<StashViewData*>();
        Q_ASSERT(data);

        // Save label
        tabLabels.append(item->text());

        // Move into the correct position
        int height = (stashPanelSize.height() + Padding) * index;
        data->getGrid()->setPos(0, height);
        index++;
    }

    emit ui->searchEdit->returnPressed();
}

void StashViewer::search(const QString &text) {
    if (ui->searchEdit->text() != text) {
        ui->searchEdit->blockSignals(true);
        ui->searchEdit->setText(text);
        ui->searchEdit->blockSignals(false);
    }
    for (int i = 0; i < ui->stashListWidget->count(); i++) {
        QListWidgetItem* item = ui->stashListWidget->item(i);

        StashViewData* data = item->data(StashViewData::ListItemDataIndex).value<StashViewData*>();
        Q_ASSERT(data);

        bool showGrid = false;
        for (QGraphicsItem* gi : data->getGrid()->childItems()) {
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
}

void StashViewer::on_searchEdit_returnPressed() {
    const QString &text = ui->searchEdit->text();
    search(text);
}

void StashViewer::on_searchEdit_textChanged(const QString &text) {
    if (text.isEmpty()) {
        search();
    }
}

void StashViewer::on_leagueBox_currentIndexChanged(const QString &text) {
    _currentLeague = text;
    emit LeagueDetailsChanged(text);
    emit RequestStashTabList(text);

    while (!_tabs.isEmpty()) {
        const QString id = _tabs.keys().first();
        StashViewData* data = _tabs.take(id);
        Q_ASSERT(data);

        ui->stashListWidget->removeItemWidget(data->getItem());
        _scene->removeItem(data->getGrid());

        delete data;
    }
    ui->stashListWidget->clear();
    _scene->clear();
    _tabs.clear();
    ui->searchEdit->clear();
}

void StashViewer::on_updateButton_clicked() {
    QList<QListWidgetItem*> items = ui->stashListWidget->selectedItems();
    for (QListWidgetItem* item : items) {
        StashViewData* data = item->data(StashViewData::ListItemDataIndex).value<StashViewData*>();
        Q_ASSERT(data);
        emit RequestStashTab(_currentLeague, data->getLocation()->hash());
    }
}
