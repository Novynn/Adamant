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
    Q_UNUSED(selectedIds);
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

QString PriceDialog::getData() const {
    if (ui->entryBook->currentIndex() == 1) {
        return ui->customEntryValue->text();
    }
    return QString("%1 %2 %3").arg(getType()).arg(getValue()).arg(getCurrency());
}

QString PriceDialog::getType() const {
    return ui->entryType->currentText();
}

double PriceDialog::getValue() const {
    return ui->entryValue->value();
}

QString PriceDialog::getCurrency() const {
    return ui->entryCurrency->currentText();
}

void PriceDialog::on_saveButton_clicked() {
    accept();
}

void PriceDialog::on_cancelButton_clicked() {
    reject();
}

void PriceDialog::on_entryType_currentIndexChanged(const QString &arg1) {
    ui->entryBook->setCurrentIndex((int)(arg1 == "Custom"));
}
