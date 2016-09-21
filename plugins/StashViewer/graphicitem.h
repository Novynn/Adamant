#ifndef GRAPHICITEM_H
#define GRAPHICITEM_H

#include <functional>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneHoverEvent>
#include <QStack>
class Item;
class ItemLocation;

class GraphicItem : public QGraphicsPixmapItem
{
public:
    enum class ShowLinkReason {
        None,
        Hover,
        Alt,
        Always
    };

    GraphicItem(QGraphicsItem* parent, const ItemLocation* location, const Item *item, const QString &imagePath);
    ~GraphicItem();

    bool IsWaitingForImage(QString imagePath=QString()) const;
    const QString getImagePath() const {
        return _imagePath;
    }

    void SetImage(QImage image);

    static QPixmap GenerateLinksOverlay(const Item* item);
    static QPixmap GenerateLinksOverlay(const Item* item, int width, int height);
    static QPair<QString, QPixmap> GenerateItemTooltip(const Item* item);

    bool IsFilteredBy(QString text);

    QString Tooltip() const {
        return _tooltipText;
    }

    bool GenerateItemTooltip();

    const Item* GetItem() const;

    void ShowLinks(bool show=true, ShowLinkReason reason=ShowLinkReason::Hover);
    void ShowTooltip(bool show=true);
protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
    QVariant itemChange(GraphicsItemChange change, const QVariant & value);
private:
    bool _waitingForImage;
    QString _imagePath;

    QGraphicsPixmapItem* _linkOverlay;
    QStack<ShowLinkReason> _linkReason;
    QGraphicsPixmapItem* _tooltip;
    QString _tooltipText;

    int _width;
    int _height;

    const Item* _item;
    const ItemLocation* _location;
};

#endif // GRAPHICITEM_H
