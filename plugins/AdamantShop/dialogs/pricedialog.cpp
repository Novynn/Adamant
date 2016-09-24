#include "pricedialog.h"
#include "ui_pricedialog.h"
#include <QLineEdit>

PriceDialog::PriceDialog(ShopList shops, QStringList selectedIds, bool tab, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PriceDialog)
    , _shops(shops) {
    ui->setupUi(this);

    for (const Shop* shop : _shops.values()) {
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);

        auto edit = new QLineEdit;
        QString value;
        for (QString id : selectedIds) {
            QString selectedValue = tab ? shop->getTabData(id).getData() : shop->getItemData(id).getData();

            if (value.isEmpty() || selectedValue == value) {
                value = selectedValue;
            }
            else {
                value = "...";
                break;
            }
        }
        edit->setText(value);

        auto item = new QTableWidgetItem(shop->name());
        item->setData(Qt::UserRole + 1, QVariant::fromValue<void*>((void*)shop));

        ui->tableWidget->setVerticalHeaderItem(row, item);
        ui->tableWidget->setCellWidget(row, 0, edit);
    }

    ui->tableWidget->verticalHeader()->show();
}

PriceDialog::~PriceDialog() {
    delete ui;
}

QHash<Shop*, QString> PriceDialog::getValues() {
    QHash<Shop*, QString> result;
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        Shop* shop = static_cast<Shop*>(ui->tableWidget->verticalHeaderItem(i)->data(Qt::UserRole + 1).value<void*>());
        if (shop) {
            QLineEdit* edit = dynamic_cast<QLineEdit*>(ui->tableWidget->cellWidget(i, 0));
            if (edit) {
                result.insert(shop, edit->text());
            }
        }
    }
    return result;
}

void PriceDialog::on_saveButton_clicked() {
    accept();
}

void PriceDialog::on_clearButton_clicked() {
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        QLineEdit* edit = dynamic_cast<QLineEdit*>(ui->tableWidget->cellWidget(i, 0));
        if (edit) {
            edit->clear();
        }
    }
}
