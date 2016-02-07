#ifndef PLUGINITEM_H
#define PLUGINITEM_H

#include <QWidget>
#include "adamantplugininfo.h"

namespace Ui {
class PluginItem;
}

class PluginItem : public QWidget
{
    Q_OBJECT

public:
    explicit PluginItem(QWidget *parent = 0);
    ~PluginItem();

    void setVersionString(const QString &version);
    void setState(PluginState state);
    void setText(const QString &text);
    void setSelected(bool selected = true);
private:
    Ui::PluginItem *ui;
};

#endif // PLUGINITEM_H
