#include "loginsessioniddialog.h"
#include "ui_loginsessioniddialog.h"
#include <QDesktopServices>
#include <QUrlQuery>
#include <QUuid>
#include <core.h>
#include <session/sessionrequest.h>

LoginSessionIdDialog::LoginSessionIdDialog(QWidget *parent, CoreService* core)
    : ILoginDialog(parent)
    , ui(new Ui::LoginSessionIdDialog)
    , _core(core)
{
    ui->setupUi(this);
    ui->errorLabel->hide();
}

void LoginSessionIdDialog::showError(const QString &error) {
    ui->errorLabel->show();
    ui->errorLabel->setText(error);
    setEnabled(true);
}

int LoginSessionIdDialog::exec() {
    setEnabled(true);
    return QDialog::exec();
}

LoginSessionIdDialog::~LoginSessionIdDialog()
{
    delete ui;
}

void LoginSessionIdDialog::on_cancelButton_clicked() {
    ui->errorLabel->hide();
    reject();
}

void LoginSessionIdDialog::on_loginButton_clicked() {
    ui->errorLabel->hide();
    _core->request()->loginWithSessionId(ui->sessionLineEdit->text());
    setEnabled(false);
}
