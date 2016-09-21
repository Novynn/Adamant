#include "node.h"
#include <QPainter>

#include <QDebug>

QHash<QString, NodeAsset*> NodeAsset::_nodeAssetHash;
QHash<QString, QPixmap> NodeAsset::_nodeAssetPixmapHash;

Node::Node(const QJsonObject& source, bool root)
    : _flags(root ? Root : None)
    , _active(false) {
    _id         = source.value("id").toInt();
    _group      = source.value("g").toInt();
    _orbit      = source.value("o").toInt();
    _orbitIndex = source.value("oidx").toInt();

    if (source.value("not").toBool())
        _flags = _flags | Notable;

    if (source.value("ks").toBool())
        _flags = _flags | KeyStone;

    if (source.value("m").toBool())
        _flags = _flags | Mastery;

    if (source.value("isJewelSocket").toBool())
        _flags = _flags | JewelSocket;

    if (source.value("isAscendancyStart").toBool())
        _flags = _flags | AscendancyRoot;

    if (source.value("isMultipleChoice").toBool())
        _flags = _flags | MultiChoiceRoot;

    if (source.value("isMultipleChoiceOption").toBool())
        _flags = _flags | MultiChoiceOption;

    _name       = source.value("dn").toString();
    _icon       = source.value("icon").toString();
    _ascendancy = source.value("ascendancyName").toString();
    _detail     = source.value("sd").toVariant().toStringList();

    _grantedPassivePoints   = source.value("passivePointsGranted").toInt();
    _grantedStrength        = source.value("sa").toInt();
    _grantedIntelligence    = source.value("ia").toInt();
    _grantedDexterity       = source.value("da").toInt();

    auto outNodesArray = source.value("out").toArray();
    for (const QJsonValue &val : outNodesArray) {
        _outNodes << val.toInt();
    }

    if (hasNodeFlag(AscendancyRoot)) {
        setZValue(1.0f);
        _pixmap = QPixmap(":/static/PassiveSkillScreenAscendancyMiddle.png");
    }
    else {
        setZValue(2.0f);

        NodeAsset* asset = NodeAsset::getAsset(_icon);
        if (asset) {
            _pixmap = asset->getPixmap(this);

            if (!hasNodeFlag(Mastery)) {
                QPixmap frame;
                if (hasNodeFlag(Notable)) {
                    frame = QPixmap(_ascendancy.isEmpty() ? ":/static/NotableFrameUnallocated.png" : ":/static/PassiveSkillScreenAscendancyFrameLargeNormal.png");
                }
                else if (hasNodeFlag(KeyStone)) {
                    frame = QPixmap(":/static/KeystoneFrameUnallocated.png");
                }
                else if (hasNodeFlag(JewelSocket)) {
                    frame = QPixmap(":/static/JewelFrameUnallocated.png");
                }
                else {
                    frame = QPixmap(_ascendancy.isEmpty() ? ":/static/Skill_Frame_Unallocated.png" : ":/static/PassiveSkillScreenAscendancyFrameSmallNormal.png");
                }
                QPixmap result(frame.width(), frame.height());
                result.fill(Qt::transparent);
                QPainter painter(&result);
                painter.drawPixmap((frame.width() / 2) - (_pixmap.width() / 2),
                                   (frame.height() / 2) - (_pixmap.height() / 2),
                                   _pixmap);
                painter.drawPixmap(frame.rect(), frame);
                _pixmap = result;
            }
        }
        else {
            qDebug() << "Failed to find asset for " << _icon;
        }
    }
}

void Node::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->drawPixmap(boundingRect().toRect(), _pixmap);
}

QRectF Node::boundingRect() const {
    return QRectF(_pixmap.rect()).translated(-_pixmap.rect().width()/2, -_pixmap.rect().height()/2);
}

Node::NodeFlags operator |(Node::NodeFlags lhs, Node::NodeFlags rhs) {
    return static_cast<Node::NodeFlags>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

QPixmap NodeAsset::getPixmap(const Node* node, bool active) const {
    QString key = "";
    //  where file is spritesheet
    if (node->hasNodeFlag(Node::Mastery)) {
        key = "mastery";
    }
    else if (node->hasNodeFlag(Node::Notable)) {
        key = active ? "notableActive" : "notableInactive";
    }
    else if (node->hasNodeFlag(Node::KeyStone)) {
        key = active ? "keystoneActive" : "keystoneInactive";
    }
    else {
        key = active ? "normalActive" : "normalInactive";
    }

    auto list = _coords.values(key);
    QPair<QString, QRect> data = list.first();
    QPixmap pixmap = _nodeAssetPixmapHash.value(data.first);
    return pixmap.copy(data.second);
}

NodeAsset* NodeAsset::getAsset(const QString& file) {
    return _nodeAssetHash.value(file);
}

void NodeAsset::addPixmapEntry(const QJsonObject& entry) {
    for (const QString &key : entry.keys()) {
        QJsonArray qualityArray = entry.value(key).toArray();
        for (const QJsonValue &val : qualityArray) {
            const QJsonObject qualityObject = val.toObject();
            const QString file = qualityObject.value("filename").toString();

            if (!_nodeAssetPixmapHash.contains(file)) {
                QPixmap pixmap(":/assets/" + file);
                _nodeAssetPixmapHash.insert(file, pixmap);
            }

            QJsonObject coords = qualityObject.value("coords").toObject();
            for (const QString &fileEntryKey : coords.keys()) {
                const QJsonObject fileEntryObject = coords.value(fileEntryKey).toObject();

                NodeAsset* asset = _nodeAssetHash.value(fileEntryKey, nullptr);
                if (asset == nullptr) {
                    asset = new NodeAsset;
                    asset->_file = file;
                    _nodeAssetHash.insert(fileEntryKey, asset);
                }

                QPair<QString, QRect> data;
                data.first = file;
                data.second = QRect(fileEntryObject.value("x").toInt(),
                             fileEntryObject.value("y").toInt(),
                             fileEntryObject.value("w").toInt(),
                             fileEntryObject.value("h").toInt());
                asset->_coords.insert(key, data);
            }
        }
    }
}
