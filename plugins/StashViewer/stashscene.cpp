#include "stashscene.h"
#include "graphicitem.h"

#include <QToolTip>
#include <QDebug>

StashScene::StashScene(QObject* parent)
    : QGraphicsScene(parent) {
    setItemIndexMethod(NoIndex);
}

void StashScene::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Alt) {
        event->accept();
        for (QGraphicsItem* item : this->items()) {
            GraphicItem* gItem = dynamic_cast<GraphicItem*>(item);
            if (gItem) {
                gItem->ShowLinks(true, GraphicItem::ShowLinkReason::Alt);
            }
        }
    }
    QGraphicsScene::keyPressEvent(event);
}

void StashScene::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Alt) {
        event->accept();
        for (QGraphicsItem* item : this->items()) {
            GraphicItem* gItem = dynamic_cast<GraphicItem*>(item);
            if (gItem) {
                gItem->ShowLinks(false, GraphicItem::ShowLinkReason::Alt);
            }
        }

    }
    QGraphicsScene::keyReleaseEvent(event);
}

void StashScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        event->accept();
        return;
    }
    QGraphicsScene::mousePressEvent(event);
}

void StashScene::focusOutEvent(QFocusEvent *event) {
    for (QGraphicsItem* item : this->items()) {
        GraphicItem* gItem = dynamic_cast<GraphicItem*>(item);
        if (gItem) {
            gItem->ShowLinks(false, GraphicItem::ShowLinkReason::Alt);
        }
    }
    QGraphicsScene::focusOutEvent(event);
}
