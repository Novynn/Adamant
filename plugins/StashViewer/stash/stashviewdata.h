#ifndef STASHVIEWDATA_H
#define STASHVIEWDATA_H

#include <QGraphicsPixmapItem>
#include <QListWidget>
#include <QObject>
#include <items/stashitemlocation.h>

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

    const StashItemLocation* getTab() const {
        return dynamic_cast<const StashItemLocation*>(_tab);
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

    void setLoaded(bool loaded=true, bool throttled=false) {
        const QString colorHint = (getTab()->tabColor().lightnessF() > 0.5) ? "light" : "dark";
        QString icon = "";
        if (loaded) {
            icon = getTab()->state() == StashItemLocation::LoadedFromDisk ? "database" : "cloud-check";
            _gridRequirement->hide();

            const StashItemLocation* tab = dynamic_cast<const StashItemLocation*>(_tab);
            if (tab && tab->type() == StashItemLocation::Currency) {
                static QPixmap currencyGrid(":/images/tabs/StashPanelCurrency.png");
                _grid->setPixmap(currencyGrid);
            }
        }
        else {
            icon = throttled ? "pause" : "cloud-download";
            _gridRequirement->setPos(_grid->boundingRect().center() - _gridRequirement->boundingRect().center());
            _gridRequirement->show();

        }
        _item->setIcon(QIcon(QString(":/icons/%1/%2.png").arg(colorHint).arg(icon)));
    }

private:
    const ItemLocation* _tab;
    QListWidgetItem* _item;
    QGraphicsPixmapItem* _grid;
    QGraphicsTextItem* _gridRequirement;
};

Q_DECLARE_METATYPE(StashViewData*)

#endif // STASHVIEWDATA_H
