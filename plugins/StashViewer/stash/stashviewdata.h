#ifndef STASHVIEWDATA_H
#define STASHVIEWDATA_H

#include <QGraphicsPixmapItem>
#include <QListWidget>
#include <QObject>
#include <items/itemlocation.h>

class StashViewData {
public:
    static const int ListItemDataIndex = 0x0100 + 1;
    StashViewData(const ItemLocation* tab, QListWidgetItem* item, QGraphicsPixmapItem* gridItem)
        : _tab(tab)
        , _item(item)
        , _grid(gridItem)
        , _gridRequirement(new QGraphicsTextItem("This tab has not been loaded.", _grid)){
    }

    ~StashViewData() {
        delete _item;
        delete _gridRequirement;
        delete _grid;
        _tab = 0;
    }

    const ItemLocation* getLocation() const {
        return _tab;
    }

    QListWidgetItem* getItem() const {
        return _item;
    }

    QGraphicsPixmapItem* getGrid() const {
        return _grid;
    }

    void setLoaded(bool loaded=true) {
        if (loaded) {
            _gridRequirement->hide();
            //_grid->setOpacity(1.0);
        }
        else {
            _gridRequirement->show();
            _gridRequirement->setPos(_grid->boundingRect().center());
            //_grid->setOpacity(0.1);
        }
    }

private:
    const ItemLocation* _tab;
    QListWidgetItem* _item;
    QGraphicsPixmapItem* _grid;
    QGraphicsTextItem* _gridRequirement;
};

Q_DECLARE_METATYPE(StashViewData*)

#endif // STASHVIEWDATA_H
