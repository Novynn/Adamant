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
    explicit PriceDialog(Shop* shop, QStringList selectedIds, bool tab = false, QWidget *parent = 0);
    ~PriceDialog();

    double getValue();
    QString getType();
    QString getCurrency();
private slots:
    void on_saveButton_clicked();
    void on_cancelButton_clicked();
private:
    Ui::PriceDialog *ui;
    Shop* _shop;
};

#endif // PRICEDIALOG_H
