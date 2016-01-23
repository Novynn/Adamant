#ifndef STASHSCENE_H
#define STASHSCENE_H

#include <QGraphicsScene>
#include <QKeyEvent>

class StashScene : public QGraphicsScene
{
public:
    StashScene(QObject *parent = 0);
protected:
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void focusOutEvent(QFocusEvent* event);
};

#endif // STASHSCENE_H
