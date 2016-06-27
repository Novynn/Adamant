#include "loginoauthdialog.h"
#include "ui_loginoauthdialog.h"
#include <QDesktopServices>
#include <QUrlQuery>
#include <QUuid>
#include <core.h>
#include <session/sessionrequest.h>

LoginOAuthDialog::LoginOAuthDialog(QWidget *parent, CoreService* core)
    : ILoginDialog(parent)
    , ui(new Ui::LoginOAuthDialog)
    , _core(core)
{
    ui->setupUi(this);
    ui->errorLabel->hide();
}

void LoginOAuthDialog::showError(const QString &error) {
    ui->errorLabel->show();
    ui->errorLabel->setText(error);
    setEnabled(true);
}

int LoginOAuthDialog::exec() {
    setEnabled(true);

    QUrl url("https://webdev2.office.grindinggear.com/oauth/authorize");
    QUrlQuery query;
    query.addQueryItem("client_id", "test");
    query.addQueryItem("response_type", "code");
    query.addQueryItem("state", QUuid::createUuid().toString());
    query.addQueryItem("redirect_uri", "https://poe.rory.io/adamant/auth");
    url.setQuery(query);
    QDesktopServices::openUrl(url);

    return QDialog::exec();
}

LoginOAuthDialog::~LoginOAuthDialog()
{
    delete ui;
}

QString LoginOAuthDialog::getSessionId() const {
    return QString();
}

QString LoginOAuthDialog::getToken() const {
    return QString();
}

void LoginOAuthDialog::on_cancelButton_clicked() {
    ui->errorLabel->hide();
    reject();
}

void LoginOAuthDialog::on_loginButton_clicked() {
    ui->errorLabel->hide();
    _core->session()->loginWithOAuth(ui->oauthLineEdit->text());
    setEnabled(false);
}
