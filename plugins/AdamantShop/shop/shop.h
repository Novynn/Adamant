#ifndef SHOP_H
#define SHOP_H

#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaProperty>
#include <QObject>
#include <QDebug>
#include <QJsonArray>

class ShopThread {
    Q_GADGET
public:
    Q_PROPERTY(QString id MEMBER _id)
    Q_PROPERTY(QDateTime updated MEMBER _updated)
    Q_PROPERTY(QDateTime bumped MEMBER _bumped)

    inline bool operator==(const ShopThread& other){
        return _id == other._id;
    }
    inline bool operator!=(const ShopThread& other) {
        return !(*this == other);
    }
private:
    QString _id;
    QDateTime _updated;
    QDateTime _bumped;

    friend class Shop;
};

class ShopDataItem {
    Q_GADGET
public:
    Q_PROPERTY(QString id MEMBER _id)
    Q_PROPERTY(QString data MEMBER _data)
    Q_PROPERTY(QDateTime updated MEMBER _updated)
    inline bool operator==(const ShopDataItem& other){
        return _id == other._id;
    }
    inline bool operator!=(const ShopDataItem& other) {
        return !(*this == other);
    }

    enum Type {
        Item,
        Tab
    };

    virtual Type type() const = 0;
private:
    Type _type;
    QString _id;
    QString _data;
    QDateTime _updated;
    friend class Shop;
};

class ShopItem : public ShopDataItem {
    Q_GADGET
public:
    Q_PROPERTY(bool inherit MEMBER _inherit)
    Type type() const {
        return ShopDataItem::Item;
    }
private:
    bool _inherit;

    friend class Shop;
};

class ShopTab : public ShopDataItem {
    Q_GADGET
public:
    Type type() const {
        return ShopDataItem::Tab;
    }
private:
    friend class Shop;
};

typedef QList<ShopThread> ShopThreadList;
typedef QMap<QString, ShopItem> ShopItemMap;
typedef QMap<QString, ShopTab> ShopTabMap;

class Shop : public QObject
{
    Q_OBJECT
public:
    explicit Shop(QObject *parent = 0);
    Shop(const QString &name, const QString &league, QObject *parent = 0);

    const QString name() const {
        return _name;
    }

    const QString league() const {
        return _league;
    }

    Q_PROPERTY(QString name MEMBER _name)
    Q_PROPERTY(QString league MEMBER _league)
    Q_PROPERTY(QString template MEMBER _template)
    Q_PROPERTY(QDateTime created MEMBER _created)

    inline void setTabData(const ShopTab &tab) {_tabs[tab._id] = tab;}
    Q_INVOKABLE inline void setTabData(const QString &id, const QString &data, QDateTime updated = QDateTime::currentDateTime()) {
        ShopTab tab;
        tab._id = id;
        tab._data = data;
        tab._updated = updated;
        setTabData(tab);
    }

    ShopTab getTabData(const QString &id) {
        return _tabs.value(id);
    }


    inline void setItemData(const ShopItem &item) {_items[item._id] = item;}
    Q_INVOKABLE inline void setItemData(const QString &id, const QString &data, QDateTime updated = QDateTime::currentDateTime(), bool inherited = false) {
        ShopItem item;
        item._id = id;
        item._data = data;
        item._updated = updated;
        item._inherit = inherited;
        setItemData(item);
    }

    ShopItem getItemData(const QString &id) {
        return _items.value(id);
    }


    bool save(QJsonObject &object) const;
    bool load(const QJsonObject &object);
    Q_INVOKABLE inline QByteArray save() {
        QJsonObject object;
        save(object);
        QJsonDocument doc(object);
        return doc.toJson();
    }

signals:

public slots:
private:
    QString _name;
    QString _league;
    QString _template;
    QDateTime _created;
    ShopThreadList _threads;
    ShopItemMap _items;
    ShopTabMap _tabs;
};

typedef QList<Shop*> ShopList;

Q_DECLARE_METATYPE(Shop*)
Q_DECLARE_METATYPE(ShopThread*)
Q_DECLARE_METATYPE(ShopThread)
Q_DECLARE_METATYPE(ShopThreadList)
Q_DECLARE_METATYPE(ShopList)

#endif // SHOP_H
