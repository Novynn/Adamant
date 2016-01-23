#ifndef GRAPHICITEM_H
#define GRAPHICITEM_H

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneHoverEvent>
#include <QStack>
class Item;

class GraphicItem : public QGraphicsPixmapItem
{
public:
    enum ShowLinkReason {
        None,
        Hover,
        Alt,
        Always
    };

    GraphicItem(QGraphicsItem* parent, const Item *item, const QString &imagePath);

    bool IsWaitingForImage(QString imagePath) const;
    void SetImage(QImage image);

    static QPixmap GenerateLinksOverlay(const Item* item);

    bool IsFilteredBy(QString text);

    void ShowLinks(bool show=true, ShowLinkReason reason=ShowLinkReason::Hover);
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

    const Item* _item;
};

#endif // GRAPHICITEM_H
