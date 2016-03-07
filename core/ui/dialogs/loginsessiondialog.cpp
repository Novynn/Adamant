#include "loginsessiondialog.h"
#include "ui_loginsessiondialog.h"

#include <core.h>
#include <session/sessionrequest.h>

LoginSessionDialog::LoginSessionDialog(QWidget *parent, CoreService* core)
    : ILoginDialog(parent)
    , ui(new Ui::LoginSessionDialog)
    , _core(core)
    , _sessionId()
{
    ui->setupUi(this);
    ui->errorLabel->hide();
}

void LoginSessionDialog::showError(const QString &error) {
    ui->errorLabel->show();
    ui->errorLabel->setText(error);
    setEnabled(true);
}

int LoginSessionDialog::exec() {
    setEnabled(true);
    return QDialog::exec();
}

LoginSessionDialog::~LoginSessionDialog()
{
    delete ui;
}

QString LoginSessionDialog::getSessionId() const {
    return _sessionId;
}

void LoginSessionDialog::on_cancelButton_clicked() {
    ui->errorLabel->hide();
    reject();
}

void LoginSessionDialog::on_loginButton_clicked() {
    ui->errorLabel->hide();
    _core->session()->login(ui->emailLineEdit->text(),
                            ui->passwordLineEdit->text());
    setEnabled(false);
}
