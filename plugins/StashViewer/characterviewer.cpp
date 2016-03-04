#include "characterviewer.h"
#include "graphicitemfactory.h"
#include "stashscene.h"
#include "ui_characterviewer.h"
#include <widgets/characterwidget.h>
#include <QDebug>
#include <session/imagecache.h>

CharacterViewer::CharacterViewer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CharacterViewer)
    , _scene(new StashScene(this))
    , _imageCache(new ImageCache)
    , _factory(new GraphicItemFactory(_imageCache))
    , _factoryThread(new QThread(this)) {
    ui->setupUi(this);

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 4);

    connect(_imageCache, &ImageCache::onImage, this, &CharacterViewer::OnImage);

    _factory->moveToThread(_factoryThread);
    connect(_factory, &GraphicItemFactory::OnItemsReady, this, [this] (QList<GraphicItem*> items, QSet<QString> images, QVariant data) {
        QString character = data.toString();
        QGraphicsItem* parent = _characterViews.value(character);
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

    _scene->setSceneRect(0, 0, 569, 569+(47*2));
    ui->graphicsView->setScene(_scene);
}

void CharacterViewer::OnImage(const QString &path, QImage image) {
    // TODO(rory): Optimize!
    for (QGraphicsRectItem* p : _characterViews.values()) {
        for (QGraphicsItem* i : p->childItems()) {
            GraphicItem* item = dynamic_cast<GraphicItem*>(i);
            if (item && item->IsWaitingForImage(path)) {
                item->SetImage(image);
            }
        }
    }
}

CharacterViewer::~CharacterViewer() {
    delete ui;
}

void CharacterViewer::setLeagues(QStringList leagues) {
    ui->filterBox->clear();
    ui->filterBox->addItem("All");
    ui->filterBox->addItems(leagues);
}

void CharacterViewer::setCharacters(QList<Character> characters) {
    ui->listWidget->clear();
    _scene->clear();
    _locations.clear();
    _characterViews.clear();

    for (const Character &character : characters) {
        auto listItem = new QListWidgetItem();
        ui->listWidget->addItem(listItem);
        auto widget = new CharacterWidget(this, character);

        listItem->setSizeHint(QSize(listItem->sizeHint().width(), widget->sizeHint().height()));


        ui->listWidget->setItemWidget(listItem, widget);

        listItem->setData(0x0100, QVariant::fromValue<Character>(character));
        listItem->setHidden(filterItem(character.league));
    }
}

void CharacterViewer::setCharacterItems(const QString &character, CharacterItemLocation* location) {
    _locations.insert(character, location);

    QGraphicsRectItem* view = _characterViews.value(character, nullptr);
    if (view) {
        _scene->removeItem(view);
        delete view;
    }
    view = _scene->addRect(0, 0, 569, 569+(47*2));
    view->setOpacity(0.2);
    view->hide();

    _characterViews.insert(character, view);

    QMetaObject::invokeMethod(_factory,
                              "SubmitLocation",
                              Qt::QueuedConnection,
                              Q_ARG(const ItemLocation*, location),
                              Q_ARG(QVariant, character));
}

void CharacterViewer::on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous) {
    Q_UNUSED(previous)

    for (auto item : _characterViews.values()) {
        item->hide();
    }

    if (current) {
        const Character c = current->data(0x0100).value<Character>();
        auto view = _characterViews.value(c.name);
        if (view) {
            view->show();
        }
    }
}

bool CharacterViewer::filterItem(const QString &league) {
    const QString filter = ui->filterBox->currentText();
    if (filter.isEmpty() || filter == "All") return false;
    return league.compare(filter, Qt::CaseInsensitive);
}

void CharacterViewer::on_filterBox_currentIndexChanged(const QString &str) {
    for (int i = 0; i < ui->listWidget->count(); i++) {
        auto item = ui->listWidget->item(i);
        if (item) {
            const Character c = item->data(0x0100).value<Character>();
            bool hide = filterItem(c.league);
            item->setHidden(hide);
            if (hide && item->isSelected()) item->setSelected(false);
        }
    }
}
