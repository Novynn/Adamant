#include "shopviewer.h"
#include "ui_shopviewer.h"
#include "adamantshopplugin.h"
#include "stashviewer.h"
#include <core.h>

ShopViewer::ShopViewer(AdamantShopPlugin* plugin, StashViewer* viewer, QWidget *parent)
    : QWidget(parent)
    , _plugin(plugin)
    , _stashViewer(viewer)
    , ui(new Ui::ShopViewer) {
    ui->setupUi(this);

    // generate some data:
    QVector<double> x(101), y(101); // initialize with entries 0..100
    for (int i=0; i<101; ++i)
    {
      x[i] = i/50.0 - 1; // x goes from -1 to 1
      y[i] = x[i]*x[i]; // let's plot a quadratic function
    }
    // create graph and assign data to it:
    ui->plot->addGraph();
    ui->plot->graph(0)->setData(x, y);
    // give the axes some labels:
    ui->plot->xAxis->setLabel("x");
    ui->plot->yAxis->setLabel("y");
    // set axes ranges, so we see all data:
    ui->plot->xAxis->setRange(-1, 1);
    ui->plot->yAxis->setRange(0, 1);
    ui->plot->replot();

    setupStashIntegration();
}

void ShopViewer::setupStashIntegration() {
    QHBoxLayout* layout = dynamic_cast<QHBoxLayout*>(_stashViewer->headerBar()->layout());
    if (layout) {
        layout->insertWidget(0, new QPushButton("Set Tab-wide Price..."));
    }
}


ShopViewer::~ShopViewer() {
    delete ui;
}

void ShopViewer::setLeagues(const QStringList &leagues) {
    const QString selected = ui->leagueBox->currentText();
    ui->leagueBox->blockSignals(true);
    ui->leagueBox->clear();
    ui->leagueBox->addItem("All");
    ui->leagueBox->addItems(leagues);
    if (!selected.isEmpty()) {
        ui->leagueBox->setCurrentText(selected);
    }
    ui->leagueBox->blockSignals(false);
}

void ShopViewer::setShops(const QList<Shop*> shops) {
    QStringList selected;
    for (auto item : ui->listWidget->selectedItems()) {
        selected << item->text();
    }
    ui->listWidget->blockSignals(true);
    _shops = shops;
    ui->listWidget->clear();
    for (const Shop* shop : _shops) {
        auto item = new QListWidgetItem(shop->name());
        item->setData(Qt::UserRole + 1, QVariant::fromValue<Shop*>((Shop*)shop));
        ui->listWidget->addItem(item);
        if (selected.contains(shop->name())) {
            item->setSelected(true);
        }
    }
    ui->listWidget->blockSignals(false);
}


void ShopViewer::on_leagueBox_currentTextChanged(const QString &league) {
    for (int i = 0; i < ui->listWidget->count(); i++) {
        auto item = ui->listWidget->item(i);
        auto shop = item->data(Qt::UserRole + 1).value<Shop*>();
        if (shop == nullptr) continue;
        item->setHidden(league != "All" && (shop->league() != league));
    }
}

void ShopViewer::on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous) {
    Q_UNUSED(previous);
    if (current != nullptr) {
        auto shop = current->data(Qt::UserRole + 1).value<Shop*>();
        if (shop == nullptr) return;
        ui->nameLabel->setText(shop->name());
        ui->leagueLabel->setText(shop->league());
    }
}

void ShopViewer::on_viewItemsButton_clicked() {
    _stashViewer->search(ui->nameLabel->text());
    _plugin->Core()->getInterface()->window()->setPageIndex(3);
}
