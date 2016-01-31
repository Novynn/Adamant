#ifndef ITEM_H
#define ITEM_H

#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QVariant>
#include <QJsonDocument>
#include <QRegularExpression>

struct ItemSocket {
    int group;
    QString attr;
};

class Item
{
public:
    Item(QJsonObject data);

    static QString IncrementFormattedString(QString formatted) {
        QRegularExpression expr("%(?<val>\\d+)");
        QRegularExpressionMatchIterator iter = expr.globalMatch(formatted);

        int offset = 0;
        while (iter.hasNext()) {
            auto match = iter.next();
            int start = match.capturedStart("val");
            QString val = QString::number(match.captured("val").toUInt() + 1);
            int len = match.capturedLength("val");

            formatted.replace(start + offset, len, val);
            if (val.length() > len)
                ++offset;
        }

        return formatted;
    }

    static QString FormatProperty(QString name, QVariantList values, int displayMode) {
        QString text = Item::IncrementFormattedString(name);
        switch (displayMode){
            case 0: {
                for (const QVariant value : values) {
                    QVariantList valueList = value.toList();
                    const QString textValue = valueList.at(0).toString();
                    const int flag = valueList.at(1).toInt();

                    Q_UNUSED(flag)
                    text += ": " + textValue;
                }
            } break;
            case 1: {
                for (const QVariant value : values) {
                    QVariantList valueList = value.toList();
                    const QString textValue = valueList.at(0).toString();
                    const int flag = valueList.at(1).toInt();

                    Q_UNUSED(flag)
                    text = textValue + " " + text;
                }
            } break;
            case 2: {
                // A progress bar!
            } break;
            case 3: {
                for (const QVariant value : values) {
                    QVariantList valueList = value.toList();
                    const QString textValue = valueList.at(0).toString();
                    const int flag = valueList.at(1).toInt();

                    Q_UNUSED(flag)
                    text = text.arg(textValue);
                }
            } break;
        }

        return text;
    }

    QVariant Data(QString key) const {
        return _data.value(key).toVariant();
    }

    QString Dump() const {
        return QJsonDocument(_data).toJson(QJsonDocument::Indented);
    }

private:
    QJsonObject _data;
};

#endif // ITEM_H
