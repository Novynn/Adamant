#ifndef ITEMRENDERER_H
#define ITEMRENDERER_H

#include <QJsonObject>
#include <QPainter>
#include <QPixmap>
#include <QRegularExpression>
#include <ui/itemtooltip.h>

enum class FrameType {
    Normal      = 0,
    Magic       = 1,
    Rare        = 2,
    Unique      = 3,
    Gem         = 4,
    Currency    = 5
};

class ItemRenderer {
public:
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
            "#ffd700",
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
                text = ItemRenderer::incrementFormattedString(name);
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

    static QPair<QString, QPixmap> render(const QJsonObject &item) {
        QString typeLineEx = item.value("typeLine").toString();
        const QString typeLine = typeLineEx.remove(QRegularExpression("\\<.*\\>")); // Greedy
        QString nameEx = item.value("name").toString();
        const QString name = nameEx.remove(QRegularExpression("\\<.*\\>")); // Greedy

        const int talismanTier = item.value("talismanTier").toInt();
        const QStringList implicitMods = item.value("implicitMods").toVariant().toStringList();
        const QStringList explicitMods = item.value("explicitMods").toVariant().toStringList();
        const QString secDescrText = item.value("secDescrText").toString();
        const QString descrText = item.value("descrText").toString();
        const bool corrupted = item.value("corrupted").toBool();
        const bool identified = item.value("identified").toBool();
        const QStringList flavourText = item.value("flavourText").toVariant().toStringList();
        int typeIndex = item.value("frameType").toInt();
        FrameType type = static_cast<FrameType>(typeIndex);

        const QVariantList properties = item.value("properties").toVariant().toList();
        const QVariantList requirements = item.value("requirements").toVariant().toList();

        QString resultText;

        ItemTooltip tooltip;

        bool singleline = name.isEmpty();
        QString suffix = "";
        if (singleline && (type == FrameType::Rare || type == FrameType::Unique))
            suffix = "SingleLine";

        int frameHeight;

        if (singleline) {
            frameHeight = 34;
            tooltip.ui->itemNameFirstLine->hide();
            tooltip.ui->itemNameSecondLine->setAlignment(Qt::AlignCenter);
            tooltip.ui->itemHeaderLeft->setFixedSize(29, frameHeight);
            tooltip.ui->itemHeaderRight->setFixedSize(29, frameHeight);
        } else {
            frameHeight = 54;
            tooltip.ui->itemNameFirstLine->show();
            tooltip.ui->itemNameFirstLine->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
            tooltip.ui->itemNameSecondLine->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
            tooltip.ui->itemHeaderLeft->setFixedSize(44, frameHeight);
            tooltip.ui->itemHeaderRight->setFixedSize(44, frameHeight);
        }
        tooltip.ui->itemNameContainerWidget->setFixedSize(16777215, frameHeight);

        const QStringList FrameToKey = {
            "White",
            "Magic",
            "Rare",
            "Unique",
            "Gem",
            "Currency"
        };

        if (typeIndex >= FrameToKey.count()) typeIndex = 0;

        QString key = FrameToKey.at(typeIndex);

        tooltip.ui->itemHeaderLeft->setStyleSheet(("border-image: url(:/tooltip/ItemHeader" + key + suffix + "Left.png);"));
        tooltip.ui->itemNameContainerWidget->setStyleSheet(("border-image: url(:/tooltip/ItemHeader" + key + suffix + "Middle.png);"));
        tooltip.ui->itemHeaderRight->setStyleSheet(("border-image: url(:/tooltip/ItemHeader" + key + suffix + "Right.png);"));

        tooltip.ui->itemNameFirstLine->setText(name);
        tooltip.ui->itemNameSecondLine->setText(typeLine);

        const static QStringList FrameToColor = {
            "#c8c8c8",
            "#88f",
            "#ff7",
            "#af6025",
            "#1ba29b",
            "#aa9e82"
        };

        QString css = "border-image: none; font-size: 19px; color: " + FrameToColor[typeIndex];
        tooltip.ui->itemNameFirstLine->setStyleSheet(css);
        tooltip.ui->itemNameSecondLine->setStyleSheet(css);

        tooltip.ui->propertiesEdit->setStyleSheet("font-size: 15px;");

        const QString separator = "<img src=':/tooltip/ItemsSeparator" + key + ".png'>";;

        bool firstPrinted = true;
        for (const QVariant property : properties) {
            QVariantMap propertyMap = property.toMap();

            const QString name = propertyMap.value("name").toString();
            const QVariantList list = propertyMap.value("values").toList();
            const int displayMode = propertyMap.value("displayMode").toInt();

            const QString text = ItemRenderer::formatProperty(name, list, displayMode, true);

            resultText += (text) + "<br>";
            firstPrinted = false;
        }

        QStringList requirementItems;
        for (const QVariant requirement : requirements) {
            QVariantMap propertyMap = requirement.toMap();

            const QString name = propertyMap.value("name").toString();
            const QVariantList list = propertyMap.value("values").toList();
            const int displayMode = propertyMap.value("displayMode").toInt();

            const QString text = ItemRenderer::formatProperty(name, list, displayMode, true);
            requirementItems << text;
        }

        if (!requirementItems.isEmpty()) {
            if (!firstPrinted) resultText += (separator) + "<br>";
            QString comma = ItemRenderer::formatProperty(", ", {}, -3, true);
            resultText += (ItemRenderer::formatProperty("Requires ", {}, -3, true) + requirementItems.join(comma)) + "<br>";
            firstPrinted = false;
        }


        if (talismanTier > 0) {
            if (!firstPrinted) resultText += (separator) + "<br>";
            resultText += (QString("Talisman Tier: %1").arg(talismanTier)) + "\n";
            firstPrinted = false;
        }

        if (!secDescrText.isEmpty()) {
            if (!firstPrinted) resultText += (separator) + "<br>";
            resultText += ItemRenderer::formatProperty(secDescrText, {}, -4, true) + "<br>";
            firstPrinted = false;
        }

        if (!implicitMods.isEmpty() && !firstPrinted) resultText += (separator) + "<br>";
        for (const QString mod : implicitMods) {
            resultText += ItemRenderer::formatProperty(mod, {}, -1, true) + "<br>";
        }

        if (!explicitMods.isEmpty() && !firstPrinted) resultText += (separator) + "<br>";
        for (const QString mod : explicitMods) {
            resultText += ItemRenderer::formatProperty(mod, {}, -1, true) + "<br>";
        }

        if (!identified) {
            if (explicitMods.isEmpty() && !firstPrinted) resultText += (separator) + "<br>";
            resultText += ItemRenderer::formatProperty("Unidentified", {}, -2, true) + "<br>";
            firstPrinted = false;
        }

        if (corrupted) {
            if (explicitMods.isEmpty() && identified && !firstPrinted) resultText += (separator) + "<br>";
            resultText += ItemRenderer::formatProperty("Corrupted", {}, -2, true) + "<br>";
            firstPrinted = false;
        }

        if (!descrText.isEmpty()) {
           if (!firstPrinted) resultText += (separator) + "<br>";
            resultText += ItemRenderer::formatProperty(descrText, {}, -4, true) + "<br>";
            firstPrinted = false;
        }

        if (!flavourText.isEmpty()) {
            if (!firstPrinted) resultText += (separator) + "<br>";
            firstPrinted = false;
        }
        for (const QString text : flavourText) {
            if (text.isEmpty()) continue;
            resultText += ItemRenderer::formatProperty(text, {}, -5, true) + "<br>";
        }

        resultText = QString("<center>%1</center>").arg(resultText);
        tooltip.ui->propertiesEdit->setText(resultText);
        tooltip.setAttribute(Qt::WA_DontShowOnScreen);
        tooltip.show();

        qreal height = tooltip.ui->propertiesEdit->document()->size().height();
        tooltip.ui->propertiesEdit->setFixedHeight(height);
        tooltip.updateGeometry();

        QPixmap result = QPixmap(tooltip.width(), height + frameHeight);
        result.fill(QColor(0, 0, 0, 204));
        QPainter painter(&result);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.drawPixmap(0, 0, tooltip.grab());

        return {tooltip.ui->propertiesEdit->toPlainText(), result};
    }
private:
    ItemRenderer() {}
};

#endif // ITEMRENDERER_H
