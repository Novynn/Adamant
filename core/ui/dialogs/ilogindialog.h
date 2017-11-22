#ifndef ILOGINDIALOG_H
#define ILOGINDIALOG_H

#include <QDialog>

class ILoginDialog : public QDialog
{
public:
    ILoginDialog(QWidget* parent) : QDialog(parent) {
        // TODO(rory): Find the correct set of options here
//        setWindowFlags(Qt::Tool|Qt::CustomizeWindowHint);
    }
    virtual QString getMethodName() const = 0;
    virtual void showError(const QString& error) = 0;
};

#endif // ILOGINDIALOG_H
