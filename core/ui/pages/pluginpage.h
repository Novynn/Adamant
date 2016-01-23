#ifndef PLUGINPAGE_H
#define PLUGINPAGE_H

#include <QWidget>

namespace Ui {
class PluginPage;
}

class CoreService;
class PluginItemDelegate;

class PluginPage : public QWidget
{
    Q_OBJECT

public:
    explicit PluginPage(CoreService* core, QWidget *parent = 0);
    ~PluginPage();

private:
    Ui::PluginPage *ui;

    CoreService* _core;
};

#endif // PLUGINPAGE_H
