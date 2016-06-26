#ifndef PRICEDIALOG_H
#define PRICEDIALOG_H

#include <QDialog>

namespace Ui {
class PriceDialog;
}

class PriceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PriceDialog(QWidget *parent = 0);
    ~PriceDialog();

private:
    Ui::PriceDialog *ui;
};

#endif // PRICEDIALOG_H
