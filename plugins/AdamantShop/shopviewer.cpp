#include "shopviewer.h"
#include "ui_shopviewer.h"
#include "adamantshopplugin.h"
#include "stashviewer.h"
#include <QDebug>
#include <dialogs/pricedialog.h>
#include <core.h>
#include <session/session.h>
#include <session/forum/forumrequest.h>
#include <widgets/shopwidget.h>

ShopViewer::ShopViewer(AdamantShopPlugin* plugin, StashViewer* viewer, QWidget *parent)
    : QWidget(parent)
    , _plugin(plugin)
    , _stashViewer(viewer)
    , _currentShop(nullptr)
    , ui(new Ui::ShopViewer)
    , _tabWidePriceButton(new QPushButton("Set Tab-wide Price...")) {
    ui->setupUi(this);

    ui->viewWidget->hide();

    QHeaderView* headerView = ui->tableWidget->horizontalHeader();
    headerView->setSectionResizeMode(QHeaderView::Stretch);
    headerView->setSectionResizeMode(0, QHeaderView::Fixed);
    headerView->setSectionResizeMode(1, QHeaderView::Interactive);
    headerView->resizeSection(1, 96);
    headerView->resizeSection(1, 384);

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
    showShop(nullptr);

    connect(_plugin->Core()->forum(), &Session::ForumRequest::requestReady, this, [this](const ForumSubmission* submission) {
        if (!_submissions.contains(submission)) return;
        ForumSubmission* sub = (ForumSubmission*)submission;
        sub->data.insert("content", "actually no, this data!");
    });

    connect(_plugin->Core()->forum(), &Session::ForumRequest::requestError, this, [this](const ForumSubmission* submission, const QString &error) {
        if (!_submissions.contains(submission)) return;
        qDebug() << "Error!" << submission->threadId << error;
        _submissions.removeOne(submission);
        delete submission;
    });

    connect(_plugin->Core()->forum(), &Session::ForumRequest::requestFinished, this, [this](const ForumSubmission* submission) {
        if (!_submissions.contains(submission)) return;
        qDebug() << "Finished!" << submission->threadId;
        _submissions.removeOne(submission);
        delete submission;
    });

    connect(_tabWidePriceButton, &QPushButton::released, this, [this]() {
        QStringList selected = _stashViewer->getSelectedTabs();
        PriceDialog dialog(_plugin->getShops(), selected, true, this);
        dialog.setWindowTitle("Set Tab-Wide Price");

        if (dialog.exec()) {
            auto result = dialog.getValues();
            for (Shop* shop : result.uniqueKeys()) {
                QString value = result.value(shop);
                if (value == "...") continue;

                for (QString id : selected) {
                    shop->setTabData(id, value);
                }
                _plugin->saveShop(shop);
            }

            _stashViewer->update();
        }
    });
}

void ShopViewer::setupStashIntegration() {
    QHBoxLayout* layout = dynamic_cast<QHBoxLayout*>(_stashViewer->headerBar()->layout());
    if (layout) {
        layout->insertWidget(0, _tabWidePriceButton);
    }
}


ShopViewer::~ShopViewer() {
    delete ui;
}

void ShopViewer::setLeagues(const QStringList &leagues) {
    _leagues = leagues;
}

void ShopViewer::addShop(const Shop* shop) {
    auto item = new QListWidgetItem;

    auto widget = new ShopWidget(this, shop->league());
    item->setSizeHint(QSize(item->sizeHint().width(), 80));

    item->setData(Qt::UserRole + 1, QVariant::fromValue<Shop*>((Shop*)shop));
    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, widget);

    _shopListItems.insert(shop, item);

    updateShopListItem(shop);
}

void ShopViewer::updateShopListItem(const Shop* shop) {
    auto item = _shopListItems.value(shop);
    Q_ASSERT(item);
    auto widget = ui->listWidget->itemWidget(item);

    widget->setDisabled(shop->isDisabled());
    ui->listWidget->repaint();
}

void ShopViewer::showShop(const Shop* shop) {
    _currentShop = (Shop*)shop;
    if (shop == nullptr) {
        ui->viewWidget->hide();
        return;
    }
    ui->plot->setVisible(shop->hasHistory());

    ui->tableWidget->clearContents();
    while (ui->tableWidget->rowCount() > 0) {
        ui->tableWidget->removeRow(0);
    }
    for (const QString &thread : shop->threads()) {
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(thread));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem("Idle"));
        QDateTime updated = shop->threadUpdated(thread);
        QDateTime bumped = shop->threadBumped(thread);
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(updated.isNull() ? "Never" : updated.toString()));
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(bumped.isNull() ? "Never" : bumped.toString()));
    }
    updateShopListItem(shop);

    ui->viewWidget->show();
}

void ShopViewer::removeShop(Shop* shop) {
    ui->listWidget->clearSelection();


    if (!_plugin->clearShop(shop)) {
        qWarning() << "Failed to delete shop: " << shop->league();
    }
    updateShopListItem(shop);

    showShop(nullptr);
}

void ShopViewer::on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous) {
    Q_UNUSED(previous);
    if (current != nullptr) {
        auto shop = current->data(Qt::UserRole + 1).value<Shop*>();
        if (shop == nullptr) return;
        showShop(shop);
    }
}

void ShopViewer::on_viewItemsButton_clicked() {
    _stashViewer->search("League Search Stuff");
    _plugin->Core()->getInterface()->window()->setPageIndex(3);
}

void ShopViewer::on_listWidget_customContextMenuRequested(const QPoint &pos) {
    auto listItem = ui->listWidget->itemAt(pos);
    if (listItem) {
        auto shop = listItem->data(Qt::UserRole + 1).value<Shop*>();
        if (shop && !shop->isUnused()) {
            QMenu menu;
            auto deleteAction = menu.addAction("Delete " + listItem->text());
            // Execute offset so double right clicking doesn't delete
            // TODO(rory): Replace this with a dialog to confirm deletion
            if (menu.exec(ui->listWidget->mapToGlobal(pos) + QPoint(5, 0)) == deleteAction) {
                removeShop(shop);
            }
        }
    }
}

void ShopViewer::on_addThreadButton_clicked() {
    if (_currentShop == nullptr) return;
    int thread = QInputDialog::getInt(this, "Adamant - Add Thread",
                                      QString("Add a thread to your %1 shop").arg(_currentShop->league()));

    if (thread != 0) {
        // Validation?
        _currentShop->addThread(QString::number(thread));
        _currentShop->setUnused(false);
        // Save
        _plugin->saveShop(_currentShop);
        // Update view
        showShop(_currentShop);
    }
}

void ShopViewer::on_updateButton_clicked() {
    if (_currentShop == nullptr) return;
    ForumSubmission* s = new ForumSubmission();
    s->threadId = "96";
    s->data.insert("content", "test");
    _submissions << s;
    _plugin->Core()->forum()->beginRequest(s);
}
