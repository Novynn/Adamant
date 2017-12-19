#ifndef LOGINOAUTHDIALOG_H
#define LOGINOAUTHDIALOG_H

#include "ilogindialog.h"
#include <QSharedPointer>
#include <QTcpServer>

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
private:
    Ui::LoginOAuthDialog *ui;
    CoreService* _core;
    QSharedPointer<QTcpServer> _server;

    QString state;
    QString token;
};

#endif // LOGINOAUTHDIALOG_H
