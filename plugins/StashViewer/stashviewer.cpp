#include "graphicitem.h"
#include "stashviewer.h"
#include "ui_stashviewer.h"

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
    , _imageThread(new QThread(this))
    , _leagueDialog(new LeagueDialog(this))
    , _currentLeague(league)
{
    ui->setupUi(this);
    ui->splitter->setStretchFactor(0, 5);
    ui->splitter->setStretchFactor(1, 1);

    _imageCache->moveToThread(_imageThread);
    connect(_imageCache, &ImageCache::onImage, this, &StashViewer::OnImage);
    connect(_imageCache, &ImageCache::destroyed, _imageThread, &QThread::deleteLater);
    _imageThread->start();

    connect(_leagueDialog, &LeagueDialog::RequestStashTabList, [this] (QString league) {
        emit RequestStashTabList(league);
    });

    // Enable rubber band selections
    ui->graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    ui->graphicsView->setOptimizationFlags(QGraphicsView::DontAdjustForAntialiasing | QGraphicsView::IndirectPainting);
    ui->graphicsView->setScene(_scene);

//    connect(_scene, &StashScene::selectionChanged, [this] () {
//        QStringList selection;
//        for (QGraphicsItem* i : _scene->selectedItems()) {
//            GraphicItem* item = dynamic_cast<GraphicItem*>(i);
//            if (item) {
//                if (item->Tooltip().isEmpty()) {
//                    item->GenerateItemTooltip();
//                }
//                selection << item->Tooltip();
//            }
//        }

//        ui->textEdit->setHtml(selection.join("<br><br>"));
//    });
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

void StashViewer::OnLeaguesList(QStringList list) {
    _leagueDialog->SetLeagues(list);
}

void StashViewer::OnTabsList(QString league, QStringList list) {
    _leagueDialog->SetLeagueTabs(league, list);
}

void StashViewer::ShowLeagueSelectionDialog() {
    emit RequestLeaguesList();
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
    }
}

void StashViewer::on_listWidget_itemSelectionChanged() {
    QList<QListWidgetItem*> items = ui->listWidget->selectedItems();

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
    QSet<QString> images;

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

        static const bool HasLoaded = 0x01;

        if (!gridItem->data(HasLoaded).toBool()) {
            // Gather Items, and Display
            StashItemLocation* tab = _tabs.value(item->text());
            if (tab) {
                for (const Item* item : tab->items()) {
                    QString icon = item->data("icon").toString();

                    if (icon.startsWith("/")) {
                        icon.prepend("https://www.pathofexile.com");
                    }
                    QString path = _imageCache->generateFileName(icon);

                    GraphicItem* gItem = new GraphicItem(gridItem, item, path);
                    // Set to always show links
                    gItem->ShowLinks(true, GraphicItem::ShowLinkReason::Always);

                    if (_imageCache->hasLocalImage(icon)) {
                        gItem->SetImage(_imageCache->getImage(icon));
                    }
                    else {
                        images.insert(icon);
                    }
                }
            }
            gridItem->setData(HasLoaded, true);
        }
        index++;
    }

    // Request Images
    for (QString imageUrl : images) {
        _imageCache->fetchImage(imageUrl);
    }

    // Set up other stuff
    if (tabLabels.size() == 1) {
        ui->selectionLabel->setText(tabLabels.first());
    }
    else {
        ui->selectionLabel->setText(QString("%1 tabs selected.").arg(tabLabels.size()));
    }

    emit ui->lineEdit->textChanged(ui->lineEdit->text());
}

void StashViewer::on_lineEdit_textChanged(const QString &text) {
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
}

void StashViewer::on_leagueButton_clicked() {
    ShowLeagueSelectionDialog();
}
