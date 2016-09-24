#ifndef PRICEDIALOG_H
#define PRICEDIALOG_H

#include <QDialog>
#include <shop/shop.h>

namespace Ui {
class PriceDialog;
}

class PriceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PriceDialog(ShopList shops, QStringList selectedIds, bool tab = false, QWidget *parent = 0);
    ~PriceDialog();

    QHash<Shop*, QString> getValues();
private slots:
    void on_saveButton_clicked();
    void on_clearButton_clicked();
private:
    Ui::PriceDialog *ui;
    ShopList _shops;
};

#endif // PRICEDIALOG_H
