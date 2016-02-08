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

    static QString incrementFormattedString(QString formatted) {
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

    static QString formatProperty(QString name, QVariantList values, int displayMode, bool rich=false) {
        const QStringList colorsMap = {
            "#fff",
            "#88f",
            "#d20000",
            "#fff",
            "#960000",
            "#366492",
            "gold",
            "#d02090"
        };

        const QString format("<font face=\"Fontin Smallcaps\" color=\"%1\">%2%3</font>");

        QString text = name;
        if (rich && !text.isEmpty()) {
            text = format.arg("#7f7f7f", name, "");
        }
        switch (displayMode){
            case -5: {
                text = format.arg("#af6025", name, "");
            } break;
            case -4: {
                text = format.arg("#1ba29b", name, "");
            } break;
            case -3: {
                text = format.arg("#7f7f7f", name, "");
            } break;
            case -2:
            case -1: {
                text = format.arg(colorsMap.at(-displayMode)).arg(name).arg("");
            } break;
            case 0: {
                for (const QVariant value : values) {
                    QVariantList valueList = value.toList();
                    QString textValue = valueList.at(0).toString();
                    const int flag = valueList.at(1).toInt();

                    if (rich) {
                        textValue = format.arg(colorsMap.at(flag)).arg(textValue).arg("");
                    }
                    if (text.isEmpty()) {
                        text += textValue;
                    }
                    else {
                        if (rich) {
                            text = format.arg("#7f7f7f", name, ": ") + textValue;
                        }
                        else {
                            text += ": " + textValue;
                        }
                    }
                }
            } break;
            case 1: {
                for (const QVariant value : values) {
                    QVariantList valueList = value.toList();
                    QString textValue = valueList.at(0).toString();
                    const int flag = valueList.at(1).toInt();

                    if (rich) {
                        textValue = format.arg(colorsMap.at(flag)).arg(textValue).arg("");
                    }
                    text = textValue + " " + text;
                }
            } break;
            case 2: {
                // A progress bar!
            text = "=== progress bar ===";
            } break;
            case 3: {
                text = Item::incrementFormattedString(name);
                if (rich) {
                    QString formatted = format.arg("#7f7f7f").arg("%REPLACE%").arg("");
                    formatted.replace("%REPLACE%", text);
                    text = formatted;
                }
                for (const QVariant value : values) {
                    QVariantList valueList = value.toList();
                    QString textValue = valueList.at(0).toString();
                    const int flag = valueList.at(1).toInt();

                    if (rich) {
                        textValue = format.arg(colorsMap.at(flag)).arg(textValue).arg("");
                    }
                    text = text.arg(textValue);
                }
            } break;
        }

        return text;
    }

    QVariant data(QString key) const {
        return _data.value(key).toVariant();
    }

    QString dump() const {
        return QJsonDocument(_data).toJson(QJsonDocument::Indented);
    }

private:
    QJsonObject _data;
};

#endif // ITEM_H
