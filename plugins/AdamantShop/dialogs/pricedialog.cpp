#include "pricedialog.h"
#include "ui_pricedialog.h"
#include <QLineEdit>

PriceDialog::PriceDialog(Shop* shop, QStringList selectedIds, bool tab, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PriceDialog)
    , _shop(shop) {
    ui->setupUi(this);

    ui->entryType->clear();
    if (tab)    ui->entryType->addItems({"@none", "~price", "~b/o"});
    else        ui->entryType->addItems({"@none", "@inherit", "~price", "~b/o", "~c/o"});

    setWindowFlags(windowFlags() |= Qt::FramelessWindowHint);

    // TODO(rory): Figure out a good way to do this
//    double value;
//    ui->entryValue->setSpecialValueText("");
//    for (QString id : selectedIds) {
//        QString selectedValue = tab ? shop->getTabData(id).getData() : shop->getItemData(id).getData();

//        if (value.isEmpty() || selectedValue == value) {
//            value = selectedValue;
//        }
//        else {
//            ui->entryValue->setSpecialValueText("...");
//            break;
//        }
//    }
//    ui->entryValue->setValue(value);

    ui->entryType->setFocus(Qt::ActiveWindowFocusReason);
}

PriceDialog::~PriceDialog() {
    delete ui;
}

QString PriceDialog::getType() {
    return ui->entryType->currentText();
}

double PriceDialog::getValue() {
    return ui->entryValue->value();
}

QString PriceDialog::getCurrency() {
    return ui->entryCurrency->currentText();
}

void PriceDialog::on_saveButton_clicked() {
    accept();
}

void PriceDialog::on_cancelButton_clicked() {
    reject();
}
