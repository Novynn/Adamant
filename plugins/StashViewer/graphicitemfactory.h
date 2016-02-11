#ifndef GRAPHICITEMFACTORY_H
#define GRAPHICITEMFACTORY_H

#include <QObject>
#include <items/item.h>
#include <items/itemlocation.h>
#include <session/imagecache.h>
#include "graphicitem.h"

class GraphicItemFactory : public QObject
{
    Q_OBJECT
public:
    explicit GraphicItemFactory(QObject *parent, ImageCache* cache)
        : QObject(parent)
        , _imageCache(cache) {
    }

signals:
    void OnItemsReady(QList<GraphicItem*> items, void* ptr);

public slots:
    void SubmitLocation(const ItemLocation* location, void* ptr) {
        QList<GraphicItem*> items;
        for (const Item* item : location->items()) {
            QString icon = item->data("icon").toString();

            if (icon.startsWith("/")) {
                icon.prepend("https://www.pathofexile.com");
            }
            QString path = _imageCache->generateFileName(icon);

            GraphicItem* gItem = new GraphicItem(0, item, path);
            // Set to always show links
            gItem->ShowLinks(true, GraphicItem::ShowLinkReason::Always);

            if (_imageCache->hasLocalImage(icon)) {
                gItem->SetImage(_imageCache->getImage(icon));
            }
            items.append(gItem);
        }

        emit OnItemsReady(items, ptr);
    }
private:
    ImageCache* _imageCache;
};

#endif // GRAPHICITEMFACTORY_H
