#include "graphicitem.h"
#include <items/item.h>
#include <QPainter>
#include <QDebug>
#include <QCursor>
#include <QGraphicsScene>
#include <QAction>
#include <QMenu>
#include <QRegularExpression>

GraphicItem::GraphicItem(QGraphicsItem *parent, const Item* item, const QString &imagePath)
    : QGraphicsPixmapItem(parent)
    , _waitingForImage(true)
    , _imagePath(imagePath)
    , _item(item)
    , _linkOverlay(nullptr) {
    int x = item->Data("x").toInt();
    int y = item->Data("y").toInt();
    int w = item->Data("w").toInt();
    int h = item->Data("h").toInt();

    QPixmap pix = QPixmap(":/images/inventory_item_background.png", "png");
    pix = pix.scaled(w * 47, h * 47);
    setPixmap(pix);
    setX(x * 47.4645);
    setY(y * 47.4645);

    setAcceptHoverEvents(true);
    setShapeMode(BoundingRectShape);
    setCursor(QCursor(Qt::PointingHandCursor));
    setFlags(ItemIsSelectable);
}

bool GraphicItem::IsWaitingForImage(QString imagePath) const {
    return _waitingForImage && imagePath == _imagePath;
}

void GraphicItem::SetImage(QImage image) {
    _waitingForImage = false;
    QPixmap pix = QPixmap::fromImage(image);
    QPixmap result(pixmap());
    {
        QPainter painter(&result);
        painter.drawPixmap(0, 0, result.width(), result.height(), pix);
    }

    setPixmap(result);
}

const int PIXELS_PER_SLOT = 47;
const int LINKH_HEIGHT = 16;
const int LINKH_WIDTH = 38;
const int LINKV_HEIGHT = LINKH_WIDTH;
const int LINKV_WIDTH = LINKH_HEIGHT;

QPixmap GraphicItem::GenerateLinksOverlay(const Item *item) {
    int height = item->Data("h").toInt();
    int width = item->Data("w").toInt();
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
    for (QVariant socketObj : item->Data("sockets").toList()) {
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

bool GraphicItem::IsFilteredBy(QString text) {
    if (text.isEmpty()) return false;
    QStringList searchable;
    searchable << _item->Data("name").toString();
    searchable << _item->Data("typeLine").toString();
    for (QString search : searchable) {
        auto expr = QRegularExpression(text, QRegularExpression::CaseInsensitiveOption);
        if (expr.isValid() && search.contains(expr))
            return false;
    }
    return true;
}

void GraphicItem::ShowLinks(bool show, ShowLinkReason reason) {
    if (show) {
        _linkReason.push(reason);
        if (_linkOverlay == nullptr) {
            QPixmap overlay = GenerateLinksOverlay(_item);
            _linkOverlay = new QGraphicsPixmapItem(overlay, this);
        }
        _linkOverlay->show();
    }
    else if (_linkOverlay != nullptr && reason != Always) {
        _linkReason.removeAll(reason);
        if (_linkReason.isEmpty())
            _linkOverlay->hide();
    }
}

void GraphicItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    event->accept();
    ShowLinks(true, Hover);
}

void GraphicItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    event->accept();
    ShowLinks(false, Hover);
}

void GraphicItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    setSelected(true);
    // TODO(rory): Do this another time (linking in dynamically from another plugin etc.)
    QMenu menu;
    QAction *setAction = menu.addAction("Set Price...");
    QAction *clearAction = menu.addAction("Clear Price");
    menu.addSeparator();
    QAction *dumpAction = menu.addAction("Dump JSON to Console");
    QAction *selectedAction = menu.exec(event->screenPos());

    if (selectedAction != nullptr) {
        scene()->clearSelection();

        if (selectedAction == dumpAction) {
            qInfo() << qPrintable(_item->Dump());
        }
    }
}

QVariant GraphicItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) {
    if (change == ItemEnabledChange) {
        if (value.toBool())
            setOpacity(1.0);
        else
            setOpacity(0.5);
        return value;
    }
    return QGraphicsItem::itemChange(change, value);
}
