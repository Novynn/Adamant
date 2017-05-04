#ifndef LOGINOAUTHDIALOG_H
#define LOGINOAUTHDIALOG_H

#include "ilogindialog.h"

class CoreService;

namespace Ui {
class LoginOAuthDialog;
}

class LoginOAuthDialog : public ILoginDialog
{
    Q_OBJECT

public:
    explicit LoginOAuthDialog(QWidget *parent, CoreService* core);
    ~LoginOAuthDialog();

    QString getMethodName() const {
        return "OAuth";
    }

    void showError(const QString& error);
    int exec();
private slots:
    void on_cancelButton_clicked();
    void on_loginButton_clicked();

private:
    Ui::LoginOAuthDialog *ui;
    CoreService* _core;
};

#endif // LOGINOAUTHDIALOG_H
