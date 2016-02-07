#include "graphicitem.h"
#include <items/item.h>
#include <QPainter>
#include <QDebug>
#include <QCursor>
#include <QGraphicsScene>
#include <QAction>
#include <QMenu>
#include <QRegularExpression>
#include <dialogs/itemtooltip.h>

GraphicItem::GraphicItem(QGraphicsItem *parent, const Item* item, const QString &imagePath)
    : QGraphicsPixmapItem(parent)
    , _waitingForImage(true)
    , _imagePath(imagePath)
    , _linkOverlay(nullptr)
    , _tooltip(nullptr)
    , _tooltipText(QString())
    , _item(item) {
    int x = item->data("x").toInt();
    int y = item->data("y").toInt();
    int w = item->data("w").toInt();
    int h = item->data("h").toInt();

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
    int height = item->data("h").toInt();
    int width = item->data("w").toInt();
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

qreal AddSeparator(QPainter* painter, qreal y, qreal width) {
    const QPixmap separator(":/headers/ns.png");
    const qreal Height = 7.91075;
    QRectF rect(0, y, width, Height);
    painter->drawPixmap(rect.toRect(), separator.scaled(width, Height));
    return Height;
}

QPixmap GraphicItem::GenerateItemTooltip(const Item *item) {
    QString typeLineEx = item->data("typeLine").toString();
    const QString typeLine = typeLineEx.remove(QRegularExpression("\\<.*\\>")); // Greedy
    QString nameEx = item->data("name").toString();
    const QString name = nameEx.remove(QRegularExpression("\\<.*\\>")); // Greedy

    const int talismanTier = item->data("talismanTier").toInt();
    const QStringList implicitMods = item->data("implicitMods").toStringList();
    const QStringList explicitMods = item->data("explicitMods").toStringList();
    const QString secDescrText = item->data("secDescrText").toString();
    const QString descrText = item->data("descrText").toString();
    const bool corrupted = item->data("corrupted").toBool();
    const QStringList flavourText = item->data("flavourText").toStringList();
    int typeIndex = item->data("frameType").toInt();
    FrameType type = static_cast<FrameType>(typeIndex);

    const QVariantList properties = item->data("properties").toList();
    const QVariantList requirements = item->data("requirements").toList();

    QString resultText;

    ItemTooltip tooltip;

    bool singleline = name.isEmpty();
    QString suffix = "";
    if (singleline && (type == FrameType::Rare || type == FrameType::Unique))
        suffix = "SingleLine";

    if (singleline) {
        tooltip.ui->itemNameFirstLine->hide();
        tooltip.ui->itemNameSecondLine->setAlignment(Qt::AlignCenter);
        tooltip.ui->itemNameContainerWidget->setFixedSize(16777215, 34);
        tooltip.ui->itemHeaderLeft->setFixedSize(29, 34);
        tooltip.ui->itemHeaderRight->setFixedSize(29, 34);
    } else {
        tooltip.ui->itemNameFirstLine->show();
        tooltip.ui->itemNameFirstLine->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
        tooltip.ui->itemNameSecondLine->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
        tooltip.ui->itemNameContainerWidget->setFixedSize(16777215, 54);
        tooltip.ui->itemHeaderLeft->setFixedSize(44, 54);
        tooltip.ui->itemHeaderRight->setFixedSize(44, 54);
    }

    const QStringList FrameToKey = {
        "White",
        "Magic",
        "Rare",
        "Unique",
        "Gem",
        "Currency"
    };

    if (typeIndex >= FrameToKey.count()) typeIndex = 0;

    QString key = FrameToKey.at(typeIndex);

    tooltip.ui->itemHeaderLeft->setStyleSheet(("border-image: url(:/tooltip/ItemHeader" + key + suffix + "Left.png);"));
    tooltip.ui->itemNameContainerWidget->setStyleSheet(("border-image: url(:/tooltip/ItemHeader" + key + suffix + "Middle.png);"));
    tooltip.ui->itemHeaderRight->setStyleSheet(("border-image: url(:/tooltip/ItemHeader" + key + suffix + "Right.png);"));

    tooltip.ui->itemNameFirstLine->setText(name);
    tooltip.ui->itemNameSecondLine->setText(typeLine);

    const static QStringList FrameToColor = {
        "#c8c8c8",
        "#88f",
        "#ff7",
        "#af6025",
        "#1ba29b",
        "#aa9e82"
    };

    QString css = "border-image: none; font-size: 20px; color: " + FrameToColor[typeIndex];
    tooltip.ui->itemNameFirstLine->setStyleSheet(css);
    tooltip.ui->itemNameSecondLine->setStyleSheet(css);

    const QString separator = "<img src=':/tooltip/Separator" + key + ".png'>";;

    bool firstPrinted = true;
    for (const QVariant property : properties) {
        QVariantMap propertyMap = property.toMap();

        const QString name = propertyMap.value("name").toString();
        const QVariantList list = propertyMap.value("values").toList();
        const int displayMode = propertyMap.value("displayMode").toInt();

        const QString text = Item::formatProperty(name, list, displayMode, true);

        resultText += (text) + "<br>";
        firstPrinted = false;
    }

    QStringList requirementItems;
    for (const QVariant requirement : requirements) {
        QVariantMap propertyMap = requirement.toMap();

        const QString name = propertyMap.value("name").toString();
        const QVariantList list = propertyMap.value("values").toList();
        const int displayMode = propertyMap.value("displayMode").toInt();

        const QString text = Item::formatProperty(name, list, displayMode, true);
        requirementItems << text;
    }

    if (!requirementItems.isEmpty()) {
        if (!firstPrinted) resultText += (separator) + "<br>";
        QString comma = Item::formatProperty(", ", {}, -3, true);
        resultText += (Item::formatProperty("Requires ", {}, -3, true) + requirementItems.join(comma)) + "<br>";
        firstPrinted = false;
    }


    if (talismanTier > 0) {
        if (!firstPrinted) resultText += (separator) + "<br>";
        resultText += (QString("Talisman Tier: %1").arg(talismanTier)) + "\n";
        firstPrinted = false;
    }

    if (!secDescrText.isEmpty()) {
        if (!firstPrinted) resultText += (separator) + "<br>";
        resultText += Item::formatProperty(secDescrText, {}, -4, true) + "<br>";
        firstPrinted = false;
    }

    if (!implicitMods.isEmpty() && !firstPrinted) resultText += (separator) + "<br>";
    for (const QString mod : implicitMods) {
        resultText += Item::formatProperty(mod, {}, -1, true) + "<br>";
    }

    if (!explicitMods.isEmpty() && !firstPrinted) resultText += (separator) + "<br>";
    for (const QString mod : explicitMods) {
        resultText += Item::formatProperty(mod, {}, -1, true) + "<br>";
    }

    if (corrupted) {
        resultText += Item::formatProperty("Corrupted", {}, -2, true) + "<br>";
        firstPrinted = false;
    }

    if (!descrText.isEmpty()) {
       if (!firstPrinted) resultText += (separator) + "<br>";
        resultText += Item::formatProperty(descrText, {}, -4, true) + "<br>";
        firstPrinted = false;
    }

    if (!flavourText.isEmpty()) {
        if (!firstPrinted) resultText += (separator) + "<br>";
        firstPrinted = false;
    }
    for (const QString text : flavourText) {
        if (text.isEmpty()) continue;
        resultText += Item::formatProperty(text, {}, -5, true) + "<br>";
    }

    tooltip.ui->propertiesLabel->setText(QString("<center>%1</center>").arg(resultText));

    return tooltip.grab();
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

void GraphicItem::GenerateItemTooltip() {
    if (_tooltip == nullptr) {
        QPixmap data = GraphicItem::GenerateItemTooltip(_item);
        if (!data.isNull()) {
            _tooltip = new QGraphicsPixmapItem(data);
            scene()->addItem(_tooltip);
            const QPointF p = scenePos();
            _tooltip->setPos(p.x() - (data.width() / 2), p.y() - data.height());
        }
    }
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
        }
        _linkOverlay->show();
    }
    else if (_linkOverlay != nullptr && reason != ShowLinkReason::Always) {
        _linkReason.removeAll(reason);
        if (_linkReason.isEmpty())
            _linkOverlay->hide();
    }
}

void GraphicItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    event->accept();
    ShowLinks(true, ShowLinkReason::Hover);

    GenerateItemTooltip();
    if (_tooltip) _tooltip->show();
}

void GraphicItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    event->accept();
    ShowLinks(false, ShowLinkReason::Hover);
    if (_tooltip) _tooltip->hide();
}

void GraphicItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event) {
    setSelected(true);
    // TODO(rory): Do this another time (linking in dynamically from another plugin etc.)
    QMenu menu;
    QAction *setAction = menu.addAction("Set Price...");
    QAction *clearAction = menu.addAction("Clear Price");
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
        else
            setOpacity(0.5);
        return value;
    }
    return QGraphicsItem::itemChange(change, value);
}
