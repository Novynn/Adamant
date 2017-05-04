#ifndef LOGINSESSIONIDDIALOG_H
#define LOGINSESSIONIDDIALOG_H

#include "ilogindialog.h"

class CoreService;

namespace Ui {
class LoginSessionIdDialog;
}

class LoginSessionIdDialog : public ILoginDialog
{
    Q_OBJECT

public:
    explicit LoginSessionIdDialog(QWidget *parent, CoreService* core);
    ~LoginSessionIdDialog();

    QString getMethodName() const {
        return "OAuth";
    }

    void showError(const QString& error);
    int exec();
private slots:
    void on_cancelButton_clicked();
    void on_loginButton_clicked();

private:
    Ui::LoginSessionIdDialog *ui;
    CoreService* _core;
};

#endif // LOGINSESSIONIDDIALOG_H
