#ifndef SHOPVIEWER_H
#define SHOPVIEWER_H

#include <QWidget>

namespace Ui {
class ShopViewer;
}

class ShopViewer : public QWidget
{
    Q_OBJECT

public:
    explicit ShopViewer(QWidget *parent = 0);
    ~ShopViewer();

private:
    Ui::ShopViewer *ui;
};

#endif // SHOPVIEWER_H
