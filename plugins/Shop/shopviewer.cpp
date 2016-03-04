#include "shopviewer.h"
#include "ui_shopviewer.h"

ShopViewer::ShopViewer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShopViewer)
{
    ui->setupUi(this);
}

ShopViewer::~ShopViewer()
{
    delete ui;
}
