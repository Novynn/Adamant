#ifndef LOGINSESSIONDIALOG_H
#define LOGINSESSIONDIALOG_H

#include "ilogindialog.h"

class CoreService;

namespace Ui {
class LoginSessionDialog;
}

class LoginSessionDialog : public ILoginDialog
{
    Q_OBJECT

public:
    explicit LoginSessionDialog(QWidget *parent, CoreService* core);
    ~LoginSessionDialog();

    QString getMethodName() const {
        return "Session Id";
    }

    QString getSessionId() const;

    void showError(const QString& error);
    int exec();
private slots:
    void on_cancelButton_clicked();
    void on_loginButton_clicked();

private:
    Ui::LoginSessionDialog *ui;
    CoreService* _core;

    QString _sessionId;
};

#endif // LOGINSESSIONDIALOG_H
