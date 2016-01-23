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

    void SetVersionString(const QString &version);
    void SetState(PluginState state);
    void SetText(const QString &text);
    void SetSelected(bool selected = true);
private:
    Ui::PluginItem *ui;
};

#endif // PLUGINITEM_H
