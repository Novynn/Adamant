#include "graphicitem.h"
#include <items/item.h>
#include <items/itemlocation.h>
#include <QPainter>
#include <QDebug>
#include <QCursor>
#include <QGraphicsScene>
#include <QAction>
#include <QMenu>
#include <QRegularExpression>
#include <QScrollBar>
#include <QGraphicsView>
#include "itemrenderer.h"

GraphicItem::GraphicItem(QGraphicsItem *parent, const ItemLocation* location, const Item* item, const QString &imagePath)
    : QGraphicsPixmapItem(parent)
    , _waitingForImage(true)
    , _imagePath(imagePath)
    , _linkOverlay(nullptr)
    , _tooltip(nullptr)
    , _tooltipText(QString())
    , _item(item)
    , _location(location) {

    QPointF pos = location->itemPos(item);
    QSize size = location->itemSize(item);

    setPos(pos * 47.4645);
    _width = size.width();
    _height = size.height();

    static QPixmap normalBack = QPixmap(":/images/inventory_item_background.png", "png");
    static QPixmap unidentifiedBack = QPixmap(":/images/item_bg_unidentified.png", "png");

    setPixmap((item->data("identified").toBool()) ? normalBack.scaled(size * 47) : unidentifiedBack.scaled(size * 47));

    setAcceptHoverEvents(true);
    setShapeMode(BoundingRectShape);
    setCursor(QCursor(Qt::PointingHandCursor));
    setFlags(ItemIsSelectable);
}

GraphicItem::~GraphicItem()
{
    // The overlay is a child, so we should delete it ourselves
    if (_linkOverlay != nullptr)
        delete _linkOverlay;
    if (_tooltip != nullptr) {
        // The tooltip is a child of the scene, so we have to remove it manually before deleting (or do we?)
        if (_tooltip->scene() != 0) {
            _tooltip->scene()->removeItem(_tooltip);
        }
        delete _tooltip;
    }
}

bool GraphicItem::IsWaitingForImage(QString imagePath) const {
    return _waitingForImage && (imagePath.isEmpty() || imagePath == _imagePath);
}

void GraphicItem::SetImage(QImage image) {
    _waitingForImage = false;
    QPixmap pix = QPixmap::fromImage(image);
    QPixmap result(pixmap());
    {
        QPainter painter(&result);
        qreal x = (pixmap().width() / 2) - (pix.width() / 2);
        qreal y = (pixmap().height() / 2) - (pix.height() / 2);
        painter.drawPixmap(x, y, pix.width(), pix.height(), pix);
    }

    setPixmap(result);
}

const int PIXELS_PER_SLOT = 47;
const int LINKH_HEIGHT = 16;
const int LINKH_WIDTH = 38;
const int LINKV_HEIGHT = LINKH_WIDTH;
const int LINKV_WIDTH = LINKH_HEIGHT;
QPixmap GraphicItem::GenerateLinksOverlay(const Item *item) {
    int height = item->data("h").toInt();
    int width = item->data("w").toInt();
    return GenerateLinksOverlay(item, width, height);
}

QPixmap GraphicItem::GenerateLinksOverlay(const Item *item, int width, int height) {
    int socket_rows = 0;
    int socket_columns = 0;
    // this will ensure we have enough room to draw the slots
    QPixmap pixmap(width * PIXELS_PER_SLOT, height * PIXELS_PER_SLOT);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);

    static const QImage link_h(":/sockets/linkH.png");
    static const QImage link_v(":/sockets/linkV.png");
    ItemSocket prev = { 255, "-" };
    size_t i = 0;

    QList<ItemSocket> sockets;
    for (QVariant socketObj : item->data("sockets").toList()) {
        QVariantMap socketObjMap = socketObj.toMap();
        sockets.append({socketObjMap.value("group").toInt(),
                        socketObjMap.value("attr").toString()});
    }

    if (sockets.size() == 1) {
        const ItemSocket &socket = sockets.first();
        QImage socket_image(":/sockets/" + socket.attr + ".png");
        painter.drawImage(0, PIXELS_PER_SLOT * i, socket_image);
        socket_rows = 1;
        socket_columns = 1;
    } else if (sockets.size() > 1){
        for (const ItemSocket &socket : sockets) {
            bool link = (socket.group == prev.group);
            QImage socket_image(":/sockets/" + socket.attr + ".png");
            if (width == 1) {
                painter.drawImage(0, PIXELS_PER_SLOT * i, socket_image);
                if (link)
                    painter.drawImage(16, PIXELS_PER_SLOT * i - 19, link_v);
                socket_columns = 1;
                socket_rows = i + 1;
            } else /* w == 2 */ {
                int row = i / 2;
                int column = i % 2;
                if (row % 2 == 1)
                    column = 1 - column;
                socket_columns = qMax(column + 1, socket_columns);
                socket_rows = qMax(row + 1, socket_rows);
                painter.drawImage(PIXELS_PER_SLOT * column, PIXELS_PER_SLOT * row, socket_image);
                if (link) {
                    if (i == 1 || i == 3 || i == 5) {
                        // horizontal link
                        painter.drawImage(
                                    PIXELS_PER_SLOT - LINKH_WIDTH / 2,
                                    row * PIXELS_PER_SLOT + 47 / 2 - LINKH_HEIGHT / 2,
                                    link_h
                                    );
                    } else if (i == 2) {
                        painter.drawImage(
                                    PIXELS_PER_SLOT * 1.5 - LINKV_WIDTH / 2,
                                    row * PIXELS_PER_SLOT - LINKV_HEIGHT / 2,
                                    link_v
                                    );
                    } else if (i == 4) {
                        painter.drawImage(
                                    PIXELS_PER_SLOT / 2 - LINKV_WIDTH / 2,
                                    row * PIXELS_PER_SLOT - LINKV_HEIGHT / 2,
                                    link_v
                                    );
                    } else {
                    }
                }
            }

            prev = socket;
            ++i;
        }
    }

    QPixmap cropped = pixmap.copy(0, 0, PIXELS_PER_SLOT * socket_columns,
                                  PIXELS_PER_SLOT * socket_rows);

    QPixmap base(width * PIXELS_PER_SLOT, height * PIXELS_PER_SLOT);
    base.fill(Qt::transparent);
    QPainter overlay(&base);
    overlay.drawPixmap((int)(0.5*(width * PIXELS_PER_SLOT - cropped.width())),
                       (int)(0.5*(height * PIXELS_PER_SLOT - cropped.height())), cropped);

    return base;
}

QPair<QString, QPixmap> GraphicItem::GenerateItemTooltip(const Item *item) {
    QJsonObject object = item->toJson();
    return ItemRenderer::render(object);

}

bool GraphicItem::IsFilteredBy(QString text) {
    if (text.isEmpty()) return false;
    QStringList searchable;
    searchable << _item->data("name").toString();
    searchable << _item->data("typeLine").toString();
    for (QString search : searchable) {
        auto expr = QRegularExpression(text, QRegularExpression::CaseInsensitiveOption);
        if (expr.isValid() && search.contains(expr))
            return false;
    }
    return true;
}

bool GraphicItem::GenerateItemTooltip() {
    if (_tooltip == nullptr) {
        auto data = GraphicItem::GenerateItemTooltip(_item);
        if (!data.first.isEmpty() && !data.second.isNull()) {
            // Parented to the scene in order to stack on top of all other elements
            _tooltip = new QGraphicsPixmapItem(data.second);
            _tooltip->setZValue(1);
            _tooltipText = data.first;
            scene()->addItem(_tooltip);
            return true;
        }
        return false;
    }
    return true;
}

const Item* GraphicItem::GetItem() const {
    return _item;
}

void GraphicItem::ShowLinks(bool show, ShowLinkReason reason) {
    if (show) {
        _linkReason.push(reason);
        if (_linkOverlay == nullptr) {
            QPixmap overlay = GenerateLinksOverlay(_item);
            _linkOverlay = new QGraphicsPixmapItem(overlay, this);
            qreal x = (pixmap().width() / 2) - (overlay.width() / 2);
            qreal y = (pixmap().height() / 2) - (overlay.height() / 2);
            _linkOverlay->setPos(x, y);
        }
        _linkOverlay->show();
    }
    else if (_linkOverlay != nullptr && reason != ShowLinkReason::Always) {
        _linkReason.removeAll(reason);
        if (_linkReason.isEmpty())
            _linkOverlay->hide();
    }
}

void GraphicItem::ShowTooltip(bool show) {
    if (show) {
        if (GenerateItemTooltip()) {
            QPointF p = QPointF(scenePos().x() - (_tooltip->boundingRect().width() / 2) + (boundingRect().width() / 2),
                                scenePos().y() - _tooltip->boundingRect().height());
            _tooltip->setPos(p);

            if (!scene()->views().isEmpty()) {
                // Get the viewport's rect converted to scene co-ordinates
                QGraphicsView* view = scene()->views().first();
                QRect viewRect = view->rect();
                viewRect.adjust(0, 0, (view->verticalScrollBar()) ? -view->verticalScrollBar()->width() : 0,
                                      (view->horizontalScrollBar()) ? -view->horizontalScrollBar()->height() : 0);
                QRectF rect = view->mapToScene(viewRect).boundingRect();

                // Get the tooltip's rect converted to scene co-ordinates
                QRectF sceneTooltipRect = _tooltip->boundingRect();
                QPointF pos = _tooltip->scenePos();
                sceneTooltipRect.moveTo(pos);

                // Check if the tooltip needs to be adjusted
                if (!rect.contains(sceneTooltipRect)) {
                    // TODO(rory): need to make this smarter for supporting embedded gems etc...
                    if (sceneTooltipRect.topLeft().y() < rect.topLeft().y()) {
                        pos += QPointF(0, rect.topLeft().y() - sceneTooltipRect.topLeft().y());
                        sceneTooltipRect.moveTo(pos);
                    }
                    if (sceneTooltipRect.topLeft().x() < rect.topLeft().x()) {
                        pos += QPointF(rect.topLeft().x() - sceneTooltipRect.topLeft().x(), 0);
                        sceneTooltipRect.moveTo(pos);
                    }
                    if (sceneTooltipRect.topRight().x() > rect.topRight().x()) {
                        pos -= QPointF(sceneTooltipRect.topRight().x() - rect.topRight().x(), 0);
                        sceneTooltipRect.moveTo(pos);
                    }
                    _tooltip->setPos(pos);
                }
            }
            _tooltip->show();
        }

    }
    else {
        if (_tooltip) _tooltip->hide();
    }
}

void GraphicItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    event->accept();
    ShowLinks(true, ShowLinkReason::Hover);

    ShowTooltip(true);
}

void GraphicItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    event->accept();
    ShowLinks(false, ShowLinkReason::Hover);

    ShowTooltip(false);
}

void GraphicItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    setSelected(true);
    // TODO(rory): Do this another time (linking in dynamically from another plugin etc.)
    QMenu menu;
    QAction *setAction = menu.addAction("Set Price...");
    QAction *clearAction = menu.addAction("Clear Prices");
    Q_UNUSED(setAction)
    Q_UNUSED(clearAction)
    menu.addSeparator();
    QAction *dumpAction = menu.addAction("Dump JSON to Console");
    QAction *selectedAction = menu.exec(event->screenPos());

    if (selectedAction != nullptr) {
        scene()->clearSelection();

        if (selectedAction == dumpAction) {
            qInfo() << qPrintable(_item->dump());
        }
    }
}

QVariant GraphicItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) {
    if (change == ItemEnabledChange) {
        if (value.toBool())
            setOpacity(1.0);
        else {
            // Disabled
            setOpacity(0.2);
            ShowTooltip(false);
        }
    }
    if (change == ItemVisibleHasChanged) {
        if (!value.toBool())
            ShowTooltip(false);
    }
    return QGraphicsItem::itemChange(change, value);
}
