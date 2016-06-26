#include "pricedialog.h"
#include "ui_pricedialog.h"

PriceDialog::PriceDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PriceDialog)
{
    ui->setupUi(this);
}

PriceDialog::~PriceDialog()
{
    delete ui;
}
