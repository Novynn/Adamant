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

#include <ui/stashlistwidgetitem.h>

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

    // Start auto update timer
    _autoUpdateTimer = startTimer(60000);

    // Required for tab status tips
    ui->stashListWidget->setMouseTracking(true);
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
    data->getItem()->setText(data->getLocation()->header());

    // TODO(rory): There must be a better place for this
    ui->stashListWidget->sortItems();
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

void StashViewer::AddTab(const QString &league, const StashItemLocation* tab)
{
    // TODO(rory): remove these static images
    static QPixmap grid(":/images/StashPanelGrid.png");
    static QPixmap currencyGrid(":/images/tabs/StashPanelCurrency.png");
    const QString header = tab->header();


    QListWidgetItem* item = new StashListWidgetItem(header, ui->stashListWidget, tab->tabIndex());
    ui->stashListWidget->addItem(item);

    // Setup Grid Items
    QGraphicsPixmapItem* gridItem = _scene->addPixmap((tab->type() == StashItemLocation::Currency) ? currencyGrid : grid);
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

void StashViewer::LoadTab(StashViewData* data) {
    // Clear out old items
    QList<QGraphicsItem*> children = data->getGrid()->childItems();
    while (!children.isEmpty()) {
        auto item = children.takeFirst();
        auto graphicItem = dynamic_cast<GraphicItem*>(item);
        if (!graphicItem) continue;
        graphicItem->setParentItem(nullptr);
        _scene->removeItem(graphicItem);
        delete graphicItem;
    }

    data->setLoaded(false);
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

void StashViewer::timerEvent(QTimerEvent* event) {
    if (event->timerId() == _autoUpdateTimer) {
        for (auto tab : _tabs.values()) {
            if (tab->autoUpdateTick()) {
                emit RequestStashTab(_currentLeague, tab->getLocation()->hash());
            }
        }
        event->accept();
    }
}

void StashViewer::LoadTab(const QString &league, const ItemLocation* tab) {
    if (_currentLeague != league) return;
    auto data = _tabs.value(tab->hash(), nullptr);
    if (data != nullptr)
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

    int autoUpdateState = -1;
    int autoUpdateInterval = -1;
    for (QListWidgetItem* item : items) {
        StashViewData* data = item->data(StashViewData::ListItemDataIndex).value<StashViewData*>();
        Q_ASSERT(data);

        if (autoUpdateState == -1) {
            autoUpdateState = (int)(data->getAutoUpdateMax() > 0) * 2;
        }
        else if (autoUpdateState != 1 && autoUpdateState != ((int)(data->getAutoUpdateMax() > 0) * 2)) {
            autoUpdateState = 1;
        }

        if (autoUpdateInterval == -1) {
            autoUpdateInterval = data->getAutoUpdateMax();
        }
        else if (autoUpdateInterval != data->getAutoUpdateMax()){
            autoUpdateInterval = 0;
        }

        // Save label
        tabLabels.append(item->text());

        // Move into the correct position
        int height = (stashPanelSize.height() + Padding) * index;
        data->getGrid()->setPos(0, height);
        index++;
    }

    /* Resolve auto update state for selection*/ {
        bool autoUpdateBlock = ui->autoUpdate->blockSignals(true);
        bool autoUpdateBlockInterval = ui->autoUpdateInterval->blockSignals(true);
        if (autoUpdateState < 0) {
            ui->autoUpdate->setEnabled(false);
        }
        else {
            ui->autoUpdate->setEnabled(true);
            ui->autoUpdate->setCheckState((Qt::CheckState)autoUpdateState);
            ui->autoUpdateInterval->setEnabled(autoUpdateState);
        }

        if (autoUpdateInterval > 0) {
            ui->autoUpdateInterval->setSpecialValueText("");
            ui->autoUpdateInterval->setValue(autoUpdateInterval);
        }
        else {
            // Multiple different values
            ui->autoUpdateInterval->setSpecialValueText("...");
            ui->autoUpdateInterval->setValue(ui->autoUpdateInterval->minimum());
        }
        ui->autoUpdate->blockSignals(autoUpdateBlock);
        ui->autoUpdateInterval->blockSignals(autoUpdateBlockInterval);
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
        data->resetAutoUpdateIndex();
        emit RequestStashTab(_currentLeague, data->getLocation()->hash());
    }
}

void StashViewer::on_autoUpdate_toggled(bool checked) {
    ui->autoUpdateInterval->setSpecialValueText("");
    QList<QListWidgetItem*> items = ui->stashListWidget->selectedItems();
    for (QListWidgetItem* item : items) {
        StashViewData* data = item->data(StashViewData::ListItemDataIndex).value<StashViewData*>();
        Q_ASSERT(data);
        data->setAutoUpdateMax(checked ? ui->autoUpdateInterval->value(): 0);
    }

    // Set proper state
    auto b = ui->autoUpdate->blockSignals(true);
    ui->autoUpdate->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    ui->autoUpdate->blockSignals(b);
}

void StashViewer::on_autoUpdateInterval_valueChanged(int val) {
    ui->autoUpdateInterval->setSpecialValueText("");
    QList<QListWidgetItem*> items = ui->stashListWidget->selectedItems();
    for (QListWidgetItem* item : items) {
        StashViewData* data = item->data(StashViewData::ListItemDataIndex).value<StashViewData*>();
        Q_ASSERT(data);
        data->setAutoUpdateMax(val);

        // Save the stash
        emit saveStash(_currentLeague, data->getTab()->hash());
    }
    ui->autoUpdate->setCheckState(Qt::Checked);
}
