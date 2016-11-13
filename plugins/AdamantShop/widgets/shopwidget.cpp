#include "shopwidget.h"
#include "ui_shopwidget.h"

ShopWidget::ShopWidget(QWidget *parent, QString league) :
    QWidget(parent),
    ui(new Ui::ShopWidget)
{
    ui->setupUi(this);

    ui->shopLeague->setText(league);

    QPixmap pixmap(QString(":/banners/Banner%1.png").arg(league.replace(" ", "")));
    if (pixmap.isNull()) {
        pixmap = QPixmap(":/banners/BannerBackground.png");
    }

    ui->shopLeagueBanner->setScaledContents(true);
    ui->shopLeagueBanner->setPixmap(pixmap);
}

ShopWidget::~ShopWidget()
{
    delete ui;
}
