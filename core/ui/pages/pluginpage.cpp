#include "pluginpage.h"
#include "ui_pluginpage.h"
#include <core.h>
#include <pluginmanager.h>
#include <ui/widgets/pluginitemdelegate.h>

PluginPage::PluginPage(CoreService *core, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PluginPage)
    , _core(core)
{
    ui->setupUi(this);

    // ListWidget
    ui->splitter->setStretchFactor(0, 1);
    // Detail Page
    ui->splitter->setStretchFactor(1, 4);

    ui->listWidget->setItemDelegate(new PluginItemDelegate());
    {

        QListWidgetItem* item = new QListWidgetItem("Core", ui->listWidget);
        item->setData(Qt::UserRole, 0);
        ui->listWidget->addItem(item);
    }

    QList<AdamantPluginInfo*> containers = _core->GetPluginManager()->GetPluginContainers();

    for (AdamantPluginInfo* container : containers) {
        QListWidgetItem* item = new QListWidgetItem(container->name, ui->listWidget);
        item->setData(Qt::UserRole, QVariant::fromValue<AdamantPluginInfo*>(container));

        ui->listWidget->addItem(item);
    }
}

PluginPage::~PluginPage()
{
    delete ui;
}
