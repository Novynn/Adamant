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
        : _location(tab)
        , _item(item)
        , _grid(gridItem)
        , _gridRequirement(new QGraphicsTextItem("This tab has not been loaded.", _grid))
        , _autoUpdateIndex(0) {
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

    void setStatus(const QString &status) {
        const QString type = StashItemLocation::TypeToName(_tab->type());
        _item->setStatusTip(QString("%1 [%2] - %3").arg(_tab->header()).arg(type).arg(status));
    }

    void setLoaded(bool loaded=true, bool throttled=false) {
        const QString colorHint = (getTab()->tabColor().lightnessF() > 0.5) ? "light" : "dark";
        QString icon = "";

        static QPixmap currencyGrid(":/images/tabs/StashPanelCurrency.png");
        static QPixmap fragmentGrid(":/images/tabs/StashPanelFragment.png");
        static QPixmap essenceGrid (":/images/tabs/StashPanelEssence.png");
        static QPixmap quadGrid    (":/images/tabs/StashPanelQuad.png");
        if (loaded) {
            if (getTab()->state() == StashItemLocation::LoadedFromDisk) {
                icon = "database";
                setStatus("Loaded from disk");
            }
            else {
                icon = "cloud-check";
                setStatus("Up to date");
            }
            _gridRequirement->hide();

            const StashItemLocation* tab = dynamic_cast<const StashItemLocation*>(_tab);
            if (tab) {
                if (tab->type() == StashItemLocation::Currency) {
                    _grid->setPixmap(currencyGrid);
                }
                else if (tab->type() == StashItemLocation::Fragment) {
                    _grid->setPixmap(fragmentGrid);
                }
                else if (tab->type() == StashItemLocation::Essence) {
                    _grid->setPixmap(essenceGrid);
                }
                else if (tab->type() == StashItemLocation::Quad) {
                    _grid->setPixmap(quadGrid);
                }
            }
        }
        else {
            icon = throttled ? "pause" : "cloud-download";
            _gridRequirement->setPos(_grid->boundingRect().center() - _gridRequirement->boundingRect().center());
            _gridRequirement->show();
            setStatus("Updating...");

        }
        _item->setIcon(QIcon(QString(":/icons/%1/%2.png").arg(colorHint).arg(icon)));
    }

    bool autoUpdateTick() {
        if (getAutoUpdateMax() == 0) return false;
        if (++_autoUpdateIndex == getAutoUpdateMax()) {
            resetAutoUpdateIndex();
            return true;
        }
        int until = getAutoUpdateMax() - _autoUpdateIndex;
        setStatus(QString("Auto-updating in %1 %2...").arg(until).arg(until == 1 ? "minute" : "minutes"));
        return false;
    }

    void resetAutoUpdateIndex() {
        _autoUpdateIndex = 0;
    }

    void setAutoUpdateMax(int max) {
        _mutableTab->setAutoUpdateInterval(max);
        resetAutoUpdateIndex();
        if (max > 0)
            setStatus(QString("Auto-updating in %1 %2...").arg(max).arg(max == 1 ? "minute" : "minutes"));
    }

    int getAutoUpdateMax() const {
        return _tab->getAutoUpdateInterval();
    }

private:
    union {
        const ItemLocation* _location;
        const StashItemLocation* _tab;
        StashItemLocation* _mutableTab;
    };
    QListWidgetItem* _item;
    QGraphicsPixmapItem* _grid;
    QGraphicsTextItem* _gridRequirement;

    // In minutes
    int _autoUpdateIndex;
};

Q_DECLARE_METATYPE(StashViewData*)

#endif // STASHVIEWDATA_H
