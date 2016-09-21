#ifndef NODE_H
#define NODE_H

#include <QGraphicsItem>
#include <QJsonArray>
#include <QJsonObject>

class Node;

class NodeAsset {
public:
    QPixmap getPixmap(const Node* node, bool active = false) const;

    static NodeAsset* getAsset(const QString &file);
    static void addPixmapEntry(const QJsonObject &entry);
private:
    QString _file;
    QMultiHash<QString, QPair<QString, QRect>> _coords;

    static QHash<QString, NodeAsset*> _nodeAssetHash;
    static QHash<QString, QPixmap> _nodeAssetPixmapHash;
};

class Group;

class Node : public QGraphicsItem
{
public:
    Node(const QJsonObject &source, bool root = false);

    enum NodeFlags {
        None                = 0x00,
        Root                = 0x01,
        Notable             = 0x02,
        KeyStone            = 0x04,
        Mastery             = 0x08,
        JewelSocket         = 0x10,
        AscendancyRoot      = 0x20,
        MultiChoiceRoot     = 0x40,
        MultiChoiceOption   = 0x80,
    };

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QRectF boundingRect() const;

    int getId() const {
        return _id;
    }

    int getOrbit() const {
        return _orbit;
    }

    int getOrbitIndex() const {
        return _orbitIndex;
    }

    int getGroupId() const {
        return _group;
    }

    bool hasNodeFlag(NodeFlags flag) const {
        return _flags & static_cast<int>(flag);
    }

    const QString getIcon() const {
        return _icon;
    }

    const QString getAscendancy() const {
        return _ascendancy;
    }

    const QList<int> getOutNodes() const {
        return _outNodes;
    }
private:
    int _id;
    int _group;
    int _orbit;
    int _orbitIndex;

    NodeFlags _flags;

    QString _name;
    QString _icon;
    QString _ascendancy;
    QStringList _detail;

    int _grantedPassivePoints;
    int _grantedStrength;
    int _grantedIntelligence;
    int _grantedDexterity;

    QList<int> _outNodes;

    bool _active;

    QPixmap _pixmap;
};

Node::NodeFlags operator | (Node::NodeFlags lhs, Node::NodeFlags rhs);

#endif // NODE_H
