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

    QString getData() const;
    QString getType() const;
protected:
    double getValue() const;
    QString getCurrency() const;
private slots:
    void on_saveButton_clicked();
    void on_cancelButton_clicked();
    void on_entryType_currentIndexChanged(const QString &arg1);

private:
    Ui::PriceDialog *ui;
    Shop* _shop;
};

#endif // PRICEDIALOG_H
