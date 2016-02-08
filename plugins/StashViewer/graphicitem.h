#ifndef GRAPHICITEM_H
#define GRAPHICITEM_H

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneHoverEvent>
#include <QStack>
class Item;

class GraphicItem : public QGraphicsPixmapItem
{
public:
    enum class ShowLinkReason {
        None,
        Hover,
        Alt,
        Always
    };

    enum class FrameType {
        Normal      = 0,
        Magic       = 1,
        Rare        = 2,
        Unique      = 3,
        Gem         = 4,
        Currency    = 5
    };

    GraphicItem(QGraphicsItem* parent, const Item *item, const QString &imagePath);

    bool IsWaitingForImage(QString imagePath) const;
    void SetImage(QImage image);

    static QPixmap GenerateLinksOverlay(const Item* item);
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

    const Item* _item;
};

#endif // GRAPHICITEM_H
