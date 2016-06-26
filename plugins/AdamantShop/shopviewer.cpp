#include "shopviewer.h"
#include "ui_shopviewer.h"
#include "adamantshopplugin.h"
#include "stashviewer.h"
#include <QDebug>
#include <dialogs/newshopdialog.h>
#include <core.h>
#include <session/session.h>
#include <session/forum/forumrequest.h>

ShopViewer::ShopViewer(AdamantShopPlugin* plugin, StashViewer* viewer, QWidget *parent)
    : QWidget(parent)
    , _plugin(plugin)
    , _stashViewer(viewer)
    , _currentShop(nullptr)
    , ui(new Ui::ShopViewer)
    , _shopDialog(new NewShopDialog(this)) {
    ui->setupUi(this);

    // Beautiful (nvm doesn't work LOL!)
//    auto sizePolicy = ui->viewWidget->sizePolicy();
//    sizePolicy.setRetainSizeWhenHidden(true);
//    ui->viewWidget->setSizePolicy(sizePolicy);
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

    _leagues = leagues;

    // Also set leagues on the new shop dialog
    _shopDialog->setLeagues(leagues);
}

void ShopViewer::addShop(const Shop* shop, bool selected) {
    auto item = new QListWidgetItem(shop->name());
    item->setData(Qt::UserRole + 1, QVariant::fromValue<Shop*>((Shop*)shop));
    ui->listWidget->addItem(item);

    if (!_leagues.contains(shop->league(), Qt::CaseInsensitive)) {
        item->setText(item->text() + " (Invalid League)");
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
    }
    else if (selected) {
        ui->listWidget->clearSelection();
        item->setSelected(true);
        ui->listWidget->setCurrentItem(item);
    }
}

void ShopViewer::showShop(const Shop* shop) {
    _currentShop = (Shop*)shop;
    if (shop == nullptr) {
        ui->viewWidget->hide();
        return;
    }
    ui->nameLabel->setText(shop->name());
    ui->leagueLabel->setText(shop->league());
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

    ui->viewWidget->show();
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
        showShop(shop);
    }
}

void ShopViewer::on_viewItemsButton_clicked() {
    _stashViewer->search(ui->nameLabel->text());
    _plugin->Core()->getInterface()->window()->setPageIndex(3);
}

void ShopViewer::on_newShopButton_clicked() {
    _shopDialog->setBadNames(_plugin->getShops().uniqueKeys());
    if (_shopDialog->exec() == QDialog::Accepted) {
        // Create new shop
        const QString name = _shopDialog->name();
        const QString league = _shopDialog->league();
        _shopDialog->reset();

        const Shop* shop = new Shop(name, league);
        _plugin->addShop((Shop*)shop); // Goodbye const
        addShop(shop, true);
    }
}

void ShopViewer::on_listWidget_customContextMenuRequested(const QPoint &pos) {
    auto listItem = ui->listWidget->itemAt(pos);
    if (listItem) {
        QMenu* menu = new QMenu(this);
        auto deleteAction = menu->addAction("Delete " + listItem->text());
        // Execute offset so double right clicking doesn't delete
        // TODO(rory): Replace this with a dialog to confirm deletion
        if (menu->exec(ui->listWidget->mapToGlobal(pos) + QPoint(5, 0)) == deleteAction) {
            auto shop = listItem->data(Qt::UserRole + 1).value<Shop*>();
            if (shop == nullptr) return;
            ui->listWidget->removeItemWidget(listItem);
            showShop(nullptr);
            if (!_plugin->deleteShop(shop)) {
                qWarning() << "Failed to delete shop: " << shop->name();
            }
            delete listItem;
        }
        delete menu;
    }
}

void ShopViewer::on_addThreadButton_clicked() {
    if (_currentShop == nullptr) return;
    int thread = QInputDialog::getInt(this, "Adamant - Add Thread",
                                      QString("Add a thread to: %1").arg(_currentShop->name()));

    if (thread != 0) {
        // Validation?
        _currentShop->addThread(QString::number(thread));
        // Save
        _plugin->saveShop(_currentShop);
        // Update view
        showShop(_currentShop);
    }
}

void ShopViewer::on_updateButton_clicked() {
    if (_currentShop == nullptr) return;
    ForumSubmission* s = new ForumSubmission();
    s->threadId = "1440653";
    s->data.insert("content", "test");
    _submissions << s;
    _plugin->Core()->forum()->beginRequest(s);
}
