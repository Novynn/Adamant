#ifndef SHOPTEMPLATE_H
#define SHOPTEMPLATE_H

#include <QString>
#include <QVariant>
#include <external/mustache.hpp>
#include <QDebug>

class ShopTemplate {
public:
    enum Section {
        Header,
        Body,
        Footer
    };

    ShopTemplate(const QString &temp)
        : _template(nullptr) {
        setTemplate(temp);
    }
    ~ShopTemplate() {
        if (_template != nullptr)
            delete _template;
    }

    void setTemplate(const QString &temp) {
        if (_template != nullptr)
            delete _template;
        _template = new Kainjow::Mustache(temp.toStdString());

        auto sections = QList<QPair<QString, Section>>{{"header", Header}, {"body", Body}, {"footer", Footer}};
        for (auto section : sections) {
            auto sectionFunc = [this, section](const std::string &content) {
                if(_renderableSections.contains(section.second)) {
                    QString simplified = QString::fromStdString(content);
                    if (simplified.startsWith("\n")) simplified = simplified.mid(1);
                    if (simplified.endsWith("\n")) simplified = simplified.left(simplified.length() - 1);
                    return simplified.toStdString();
                }
                return std::string("");
            };
            set(section.first, sectionFunc);
        }
    }

    Kainjow::Mustache::Data resolveData(const QVariant &variant) {
        Kainjow::Mustache::Data data;

        switch (variant.type()) {
            case QVariant::Bool: {
                data = Kainjow::Mustache::Data(variant.toBool() ? Kainjow::Mustache::Data::Type::True : Kainjow::Mustache::Data::Type::False);
            } break;
            case QVariant::String: {
                data = Kainjow::Mustache::Data(variant.toString().toStdString());
            } break;
            case QVariant::StringList:
            case QVariant::List: {
                QVariantList list = variant.toList();
                std::vector<Kainjow::Mustache::Data> dest;
                for (QVariant variant : list) {
                    dest.push_back(resolveData(variant));
                }
                data = Kainjow::Mustache::Data(dest);
            } break;
            case QVariant::Map: {
                QVariantMap map = variant.toMap();
                for (const QString &key : map.uniqueKeys()) {
                    QVariant item = map.value(key);
                    data.set(key.toStdString(), resolveData(item));
                }
            } break;
            case QVariant::Hash: {
                QVariantHash hash = variant.toHash();
                for (const QString &key : hash.uniqueKeys()) {
                    QVariant item = hash.value(key);
                    data.set(key.toStdString(), resolveData(item));
                }
            } break;
            default: {
                qWarning() << "Unknown type: " << variant;
            }
        }
        return data;
    }

    void set(const QString& key, const QVariant &variant) {
        _data.set(key.toStdString(), resolveData(variant));
    }

    void set(const QString &key, const Kainjow::Mustache::Data::LambdaType &lambda) {
        _data.set(key.toStdString(), lambda);
    }

    QString render(QList<Section> sections = {}) {
        _renderableSections = sections;
        return QString::fromStdString(_template->render(_data));
    }

    bool isValid() const {
        return _template->isValid();
    }

    const QString errorMessage() const {
        return QString::fromStdString(_template->errorMessage());
    }

    void render(const std::function<void(const QString&)>& handler, QList<Section> sections = {}) {
        _renderableSections = sections;
        _template->render(_data, [handler](const std::string& part) {
            handler(QString::fromStdString(part));
        });
    }

private:
    Kainjow::Mustache* _template;
    Kainjow::Mustache::Data _data;

    QList<Section> _renderableSections;
};

#endif // SHOPTEMPLATE_H
