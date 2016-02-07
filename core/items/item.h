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

    static QString formatName(QString name, int frame, bool rich=false) {
        QStringList frameToColor = {
            "#c8c8c8",
            "#88f",
            "#ff7",
            "#af6025",
            "#1ba29b",
            "#aa9e82"
        };
        if (frame > frameToColor.count()) frame = 0;
        return QString("<font color=\"%1\">%2</font>").arg(frameToColor.at(frame), name);
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

        QString text = name;
        if (rich && !text.isEmpty()) {
            text = QString("<font color=\"%1\">%2</font>").arg("#7f7f7f", name);
        }
        switch (displayMode){
            case -5: {
                text = QString("<font color=\"%1\">%2</font>").arg("#af6025", name);
            } break;
            case -4: {
                text = QString("<font color=\"%1\">%2</font>").arg("#1ba29b", name);
            } break;
            case -3: {
                text = QString("<font color=\"%1\">%2</font>").arg("#7f7f7f", name);
            } break;
            case -2:
            case -1: {
                text = QString("<font color=\"%1\">%2</font>").arg(colorsMap.at(-displayMode)).arg(name);
            } break;
            case 0: {
                for (const QVariant value : values) {
                    QVariantList valueList = value.toList();
                    QString textValue = valueList.at(0).toString();
                    const int flag = valueList.at(1).toInt();

                    if (rich) {
                        textValue = QString("<font color=\"%1\">%2</font>").arg(colorsMap.at(flag)).arg(textValue);
                    }
                    if (text.isEmpty()) {
                        text += textValue;
                    }
                    else {
                        if (rich) {
                            text = QString("<font color=\"%1\">%2: </font>").arg("#7f7f7f", name) + textValue;
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
                        textValue = QString("<font color=\"%1\">%2</font>").arg(colorsMap.at(flag)).arg(textValue);
                    }
                    text = textValue + " " + text;
                }
            } break;
            case 2: {
                // A progress bar!
            text = "{{ progress bar }}";
            } break;
            case 3: {
                text = Item::incrementFormattedString(name);
                if (rich) {
                    text = QString("<font color=\"%1\">%2</font>").arg("#7f7f7f").arg(text);
                }
                for (const QVariant value : values) {
                    QVariantList valueList = value.toList();
                    QString textValue = valueList.at(0).toString();
                    const int flag = valueList.at(1).toInt();

                    if (rich) {
                        textValue = QString("<font color=\"%1\">%2</font>").arg(colorsMap.at(flag)).arg(textValue);
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
