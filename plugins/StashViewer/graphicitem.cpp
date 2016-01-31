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
    , _linkOverlay(nullptr)
    , _tooltip(nullptr)
    , _tooltipText(QString()) {
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

qreal AddSeparator(QPainter* painter, qreal y, qreal width) {
    const QPixmap separator(":/headers/ns.png");
    const qreal Height = 7.91075;
    QRectF rect(0, y, width, Height);
    painter->drawPixmap(rect.toRect(), separator.scaled(width, Height));
    return Height;
}

QPair<QPixmap, QString> GraphicItem::GenerateItemTooltip(const Item *item) {
    QString typeLineEx = item->Data("typeLine").toString();
    const QString typeLine = typeLineEx.remove(QRegularExpression("\\<.*\\>")); // Greedy
    QString nameEx = item->Data("name").toString();
    const QString name = nameEx.remove(QRegularExpression("\\<.*\\>")); // Greedy

    const int talismanTier = item->Data("talismanTier").toInt();
    const QStringList implicitMods = item->Data("implicitMods").toStringList();
    const QStringList explicitMods = item->Data("explicitMods").toStringList();
    const QString secDescrText = item->Data("secDescrText").toString();
    const QString descrText = item->Data("descrText").toString();
    const bool corrupted = item->Data("corrupted").toBool();
    const QStringList flavourText = item->Data("flavourText").toStringList();
    FrameType type = static_cast<FrameType>(item->Data("frameType").toInt());

    const QVariantList properties = item->Data("properties").toList();
    const QVariantList requirements = item->Data("requirements").toList();

    QString resultText;

    // em calculations
//    const qreal FontSize = 14.3;

//    qreal runningHeight = 0.0;

    const int MaxHeight = 800; // This will be trimmed once the drawing is done
    QPixmap result = QPixmap(314, MaxHeight);
    result.fill(Qt::transparent);
//    QPainter painter(&result);
//    painter.setRenderHint(QPainter::HighQualityAntialiasing);
    switch (type) {
        case FrameType::Normal: {
        /*
            const int normalFrameHeaderHeight = 33;
            const int normalFrameHeaderWidth = 314; // TODO(rory): This should be dynamic!!!
            const QRectF headerRect = QRectF(0, 0, normalFrameHeaderWidth, normalFrameHeaderHeight);

            // Draw Header Image
            {
                const QPixmap nl = QPixmap(":/headers/nl.png");
                const QPixmap nm = QPixmap(":/headers/nm.png");
                const QPixmap nr = QPixmap(":/headers/nr.png");
                painter.save();
                painter.drawPixmap(0, 0,
                                   nl.width(), headerRect.height(), nl);
                painter.drawPixmap(nl.width(), 0,
                                   headerRect.width() - (nl.width() + nr.width()), headerRect.height(), nm);
                painter.drawPixmap(headerRect.width() - nr.width(), 0,
                                   nr.width(), headerRect.height(), nr);
                painter.restore();
            }
            runningHeight += normalFrameHeaderHeight;

            // Draw Type Line
            {
                painter.save();
                painter.setPen(QColor(0xc8, 0xc8, 0xc8));
                painter.setBrush(QBrush(Qt::transparent));
                painter.setRenderHint(QPainter::TextAntialiasing);
                painter.setFont(QFont("Fontin SmallCaps", FontSize, QFont::Medium));
                painter.drawText(headerRect.adjusted(7.23428, 30, -7.23428, -30),
                                 typeLine, QTextOption(Qt::AlignCenter));
                painter.restore();
            }

            // Test Draw Bounds
            {
                painter.save();
                QColor back(Qt::black);
                back.setAlpha(204); // Extracted from the image pathofexile.com uses
                painter.setBrush(back);
                painter.drawRect(0, normalFrameHeaderHeight, normalFrameHeaderWidth, MaxHeight);
                painter.restore();
            }
            runningHeight += 0.4 * FontSize; // 0.4em

            // Contents
            {
                const qreal LineXOffset = 6.08519;
                const qreal LineWidth = normalFrameHeaderWidth - LineXOffset;
                // Draw Talisman Stuff
                if (talismanTier > 0){
                    const QString text = QString("Talisman Tier: %1").arg(talismanTier);
                    painter.save();
                    painter.setPen(QColor(0xc8, 0xc8, 0xc8));
                    painter.setBrush(QBrush(Qt::transparent));
                    painter.setRenderHint(QPainter::TextAntialiasing);
                    painter.setFont(QFont("Fontin SmallCaps", FontSize - 3, QFont::Normal));
                    painter.drawText(QRectF(LineXOffset, runningHeight, LineWidth, 18),
                                     text, QTextOption(Qt::AlignCenter));
                    painter.restore();
                    runningHeight += 18;
                    runningHeight += AddSeparator(&painter, runningHeight, normalFrameHeaderWidth);
                }
                // Draw Mods
                for (const QString mod : implicitMods) {
                    painter.save();
                    painter.setPen(QColor(0x88, 0x88, 0xff));
                    painter.setBrush(QBrush(Qt::transparent));
                    painter.setRenderHint(QPainter::TextAntialiasing);
                    painter.setFont(QFont("Fontin SmallCaps", FontSize - 3, QFont::Normal));
                    painter.drawText(QRectF(LineXOffset, runningHeight, LineWidth, 18),
                                     mod, QTextOption(Qt::AlignCenter));
                    painter.restore();
                    runningHeight += 18;
                }

                if (corrupted) {
                    const QString text("Corrupted");
                    painter.save();
                    painter.setPen(QColor(0xd2, 0x00, 0x00));
                    painter.setBrush(QBrush(Qt::transparent));
                    painter.setRenderHint(QPainter::TextAntialiasing);
                    painter.setFont(QFont("Fontin SmallCaps", FontSize - 3, QFont::Normal));
                    painter.drawText(QRectF(LineXOffset, runningHeight, LineWidth, 18),
                                     text, QTextOption(Qt::AlignCenter));
                    painter.restore();
                    runningHeight += 18;
                }
                if (implicitMods.count() > 0) {
                    runningHeight += AddSeparator(&painter, runningHeight, normalFrameHeaderWidth);
                }

                // Draw Flavour Text
                for (const QString text : flavourText) {
                    if (text.isEmpty()) continue;
                    painter.save();
                    painter.setPen(QColor(0xaf, 0x60, 0x25));
                    painter.setBrush(QBrush(Qt::transparent));
                    painter.setRenderHint(QPainter::TextAntialiasing);
                    painter.setFont(QFont("Fontin SmallCaps", FontSize - 4, QFont::Normal, true));
                    painter.drawText(QRectF(LineXOffset, runningHeight, LineWidth, 18),
                                     text, QTextOption(Qt::AlignCenter));
                    painter.restore();
                    runningHeight += 18;
                }
            }

            runningHeight += 0.5 * FontSize; // 0.5em
            painter.end();

            result = result.copy(0, 0, normalFrameHeaderWidth, runningHeight);

            */
        } break;
        case FrameType::Magic: {

        } break;
        case FrameType::Rare: {
            resultText += (name) + "\n";
        } break;
        case FrameType::Unique: {
            resultText += (name) + "\n";
        } break;
        case FrameType::Gem: {

        } break;
        case FrameType::Currency: {

        } break;
        default: {

        } break;
    }
    resultText += (typeLine) + "\n";

    const QString separator = QString('-').repeated(20);

    resultText += (QString('=').repeated(20)) + "\n";
    bool firstPrinted = true;

    // if (!properties.isEmpty()) qDebug() << qPrintable(separator);
    for (const QVariant property : properties) {
        QVariantMap propertyMap = property.toMap();

        const QString name = propertyMap.value("name").toString();
        const QVariantList list = propertyMap.value("values").toList();
        const int displayMode = propertyMap.value("displayMode").toInt();

        const QString text = Item::FormatProperty(name, list, displayMode);

        resultText += (text) + "\n";
        firstPrinted = false;
    }

    QStringList requirementItems;
    for (const QVariant requirement : requirements) {
        QVariantMap propertyMap = requirement.toMap();

        const QString name = propertyMap.value("name").toString();
        const QVariantList list = propertyMap.value("values").toList();
        const int displayMode = propertyMap.value("displayMode").toInt();

        const QString text = Item::FormatProperty(name, list, displayMode);
        requirementItems << text;
    }

    if (!requirementItems.isEmpty()) {
        if (!firstPrinted) resultText += (separator) + "\n";
        resultText += ("Requires " + requirementItems.join(", ")) + "\n";
        firstPrinted = false;
    }


    if (talismanTier > 0) {
        if (!firstPrinted) resultText += (separator) + "\n";
        resultText += (QString("Talisman Tier: %1").arg(talismanTier)) + "\n";
        firstPrinted = false;
    }

    if (!secDescrText.isEmpty()) {
        if (!firstPrinted) resultText += (separator) + "\n";
        resultText += (secDescrText) + "\n";
        firstPrinted = false;
    }

    if (!implicitMods.isEmpty() && !firstPrinted) resultText += (separator) + "\n";
    for (const QString mod : implicitMods) {
        resultText += (mod) + "\n";
    }

    if (!explicitMods.isEmpty() && !firstPrinted) resultText += (separator) + "\n";
    for (const QString mod : explicitMods) {
        resultText += (mod) + "\n";
    }

    if (corrupted) {
        resultText += "Corrupted\n";
        firstPrinted = false;
    }

    if (!descrText.isEmpty()) {
       if (!firstPrinted) resultText += (separator) + "\n";
        resultText += (descrText) + "\n";
        firstPrinted = false;
    }

    if (!flavourText.isEmpty()) {
        if (!firstPrinted) resultText += (separator) + "\n";
        firstPrinted = false;
    }
    for (const QString text : flavourText) {
        if (text.isEmpty()) continue;
        resultText += (text) + "\n";
    }

    return QPair<QPixmap, QString>(result, resultText);
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

void GraphicItem::GenerateItemTooltip() {
    if (_tooltip == nullptr) {
        QPair<QPixmap, QString> data = GraphicItem::GenerateItemTooltip(_item);
        if (!data.first.isNull()) {
            _tooltip = new QGraphicsPixmapItem(data.first);
            _tooltipText = data.second;
            scene()->addItem(_tooltip);
            // const QPointF p = mapToScene(pos());
            _tooltip->setPos(-_tooltip->boundingRect().width(), 0);
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
    _tooltip->show();
}

void GraphicItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    event->accept();
    ShowLinks(false, ShowLinkReason::Hover);
    _tooltip->hide();
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
