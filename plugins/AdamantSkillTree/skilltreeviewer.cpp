#include "skilltreeviewer.h"
#include "ui_skilltreeviewer.h"
#include "adamantskilltreeplugin.h"
#include "node.h"

#include <QFile>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QtMath>
#include <qgraphicsitem.h>
#include <QMouseEvent>
#include <QOpenGLWidget>

struct Group;

enum class CharacterClass {
    None = -1,
    Scion,
    Marauder,
    Ranger,
    Witch,
    Duelist,
    Templar,
    Shadow
};

struct NodeImage;

struct NodeImageMipmap {
    QMap<int, NodeImage*> mipmap;
};

struct NodeImage {
    QRect active;
    QRect inactive;
    // TODO(rory): Replace with a table lookup
    QPixmap* activePixmap;
    QPixmap* inactivePixmap;
};

struct Group {
    int id;
    QPointF pos;
    QList<int> nodes;

    static int getOrbitRadius(int orbit) {
        switch (orbit) {
            case 0: return 0;
            case 1: return (int)(82 / 2.5f);
            case 2: return (int)(162 / 2.5f);
            case 3: return (int)(335 / 2.5f);
            case 4: return (int)(493 / 2.5f);
        }
        return -1;
    }

    QPointF getNodePos(Node* node) const {
        if (node->getOrbit() == 0)
            return pos;

        qreal indexFactor = 1.0f;
        switch (node->getOrbit()) {
            case 1: {
                indexFactor = 6.0f;
            } break;
            case 2:
            case 3: {
                indexFactor = 12.0f;
            } break;
            case 4: {
                indexFactor = 40.0f;
            } break;
        }

        qreal angle = qDegreesToRadians(((node->getOrbitIndex() / indexFactor) * 360.0f) - 90.0f);
        int x = Group::getOrbitRadius(node->getOrbit()) * qCos(angle);
        int y = Group::getOrbitRadius(node->getOrbit()) * qSin(angle);

        return pos + QPointF(x, y);
    }
};

SkillTreeViewer::SkillTreeViewer(AdamantSkillTreePlugin* plugin)
    : QWidget()
    , ui(new Ui::SkillTreeViewer)
    , _plugin(plugin)
{
    ui->setupUi(this);

//    ui->graphicsView->setViewport(new QOpenGLWidget());

    ui->graphicsView->viewport()->installEventFilter(this);

    QByteArray jsonData;
    QFile file("plugins/adamant.skilltree/data.json");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        jsonData = file.readAll();
        file.close();
    }
    else {
        qDebug() << "Could not open data.json";
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &err);
    if (err.error != QJsonParseError::NoError) {
        qDebug() << "Could not parse data.json: " << err.errorString();
    }
    QJsonObject groups = doc.object().value("groups").toObject();
    QJsonArray nodes = doc.object().value("nodes").toArray();

    int minX = doc.object().value("min_x").toInt();
    int minY = doc.object().value("min_y").toInt();
    int maxX = doc.object().value("max_x").toInt();
    int maxY = doc.object().value("max_y").toInt();


    QJsonObject skillSprites = doc.object().value("skillSprites").toObject();
    NodeAsset::addPixmapEntry(skillSprites);

    QGraphicsScene* scene = new QGraphicsScene(QRectF(QPointF(minX, minY), QPointF(maxX, maxY)));
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);

    QHash<int, Group*> knownGroups;
    QHash<int, Node*> knownNodes;
    QMap<Node*, CharacterClass> classStartNodeMap;

    for (const QString &key : groups.keys()) {
        const QJsonObject groupObject = groups.value(key).toObject();
        qreal x = groupObject.value("x").toDouble() / 2.5;
        qreal y = groupObject.value("y").toDouble() / 2.5;

        Group* group = new Group;
        group->id = key.toInt();
        group->pos = QPointF(x, y);

        knownGroups.insert(group->id, group);
    }

    for (const QJsonValue &nodeVal : nodes) {
        const QJsonObject nodeObject = nodeVal.toObject();

        QJsonArray classesStarted = nodeObject.value("spc").toArray();
        CharacterClass characterRoot = CharacterClass::None;
        if (!classesStarted.isEmpty()) {
            characterRoot = static_cast<CharacterClass>(classesStarted.first().toInt());
        }

        Node* node = new Node(nodeObject, characterRoot != CharacterClass::None);

        if (characterRoot != CharacterClass::None){
            classStartNodeMap.insert(node, characterRoot);
        }

        Group* group = knownGroups.value(node->getGroupId());
        Q_ASSERT(group);
        node->setPos(group->getNodePos(node));

        scene->addItem(node);
        knownNodes.insert(node->getId(), node);

//        QString icon = nodeObject.value("icon").toString();
//        NodeImageMipmap* mipmap = nodeMipmapImages.value(icon, nullptr);
//        NodeImage* image = mipmap->mipmap.last();

//        QPixmap pixmap = image->inactivePixmap->copy(image->inactive);
//        pixmap = pixmap.scaled(pixmap.width() * 2, pixmap.height() * 2);

//        QGraphicsPixmapItem* item = scene->addPixmap(pixmap);
//        item->setPos(pos.x() - pixmap.width()/2,
//                     pos.y() - pixmap.height()/2);
//        item->setZValue(1);

//        {
//            QGraphicsTextItem* textItem = scene->addText(QString::number(id));
//            textItem->setPos(pos.x(), pos.y());
//        }
    }

    for (const QJsonValue &nodeVal : nodes) {
        const QJsonObject nodeObject = nodeVal.toObject();
        int id = nodeObject.value("id").toInt();
        Node* node = knownNodes.value(id);
        auto group = knownGroups.value(node->getGroupId());
        auto nodePoint = group->getNodePos(node);

        for (int outNodeId : node->getOutNodes()) {
            Node* outNode = knownNodes.value(outNodeId);
            auto outGroup = knownGroups.value(outNode->getGroupId());
            auto outNodePoint = outGroup->getNodePos(outNode);

            if (node->getGroupId() == outNode->getGroupId() &&
                node->getOrbit() == outNode->getOrbit()) {
                // Angle line
                QPainterPath path;

                QLineF first(group->pos, nodePoint);
                QLineF second(group->pos, outNodePoint);
                int rectSize = Group::getOrbitRadius(node->getOrbit()) * 2;
                QRectF rect(group->pos.x() + -rectSize/2, group->pos.y() + -rectSize/2, rectSize, rectSize);

                path.moveTo(nodePoint);
                qreal firstAngle = first.angle();
                qreal secondAngle = second.angle();
                qreal angleDiff = first.angleTo(second);

                if (angleDiff > 180.0f) {
                    firstAngle = secondAngle;
                    secondAngle = first.angle();
                    angleDiff = second.angleTo(first);
                    path.moveTo(outNodePoint);
                }
                path.arcTo(rect, firstAngle, angleDiff);

                scene->addPath(path, QPen(Qt::darkGray, 5));
            }
            else if (outNode->getAscendancy() != node->getAscendancy()) {
                // NOTE(rory): Ignore linking to starting node from outside
            }
            else {
                scene->addLine(nodePoint.x(), nodePoint.y(), outNodePoint.x(), outNodePoint.y(), QPen(Qt::darkGray, 5));
            }
        }
    }

    ui->graphicsView->setScene(scene);
}

void SkillTreeViewer::wheelEvent(QWheelEvent *event){
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    double scaleFactor = 1.10;
    if(event->delta() > 0) {
        ui->graphicsView-> scale(scaleFactor, scaleFactor);

    } else {
         ui->graphicsView->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
}

bool SkillTreeViewer::eventFilter(QObject* watched, QEvent* event) {
    if (watched == ui->graphicsView->viewport() && event->type() == QEvent::Wheel) {
        return true;
    }
    return false;
}

SkillTreeViewer::~SkillTreeViewer()
{
    delete ui;
}
