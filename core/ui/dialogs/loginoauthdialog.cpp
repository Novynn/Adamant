#include "loginoauthdialog.h"
#include "ui_loginoauthdialog.h"
#include <QDesktopServices>
#include <QStringList>
#include <QTcpServer>
#include <QUrlQuery>
#include <QUuid>
#include <core.h>
#include <session/sessionrequest.h>

LoginOAuthDialog::LoginOAuthDialog(QWidget *parent, CoreService* core)
    : ILoginDialog(parent)
    , ui(new Ui::LoginOAuthDialog)
    , _core(core)
    , _server(QSharedPointer<QTcpServer>::create(this))
{
    ui->setupUi(this);
    ui->errorLabel->hide();

    _server->listen(QHostAddress::LocalHost);
    qDebug() << "Listening on localhost port" << _server->serverPort();

    connect(_server.data(), &QTcpServer::newConnection, this, [this](){
        auto socket = _server->nextPendingConnection();
        socket->waitForConnected();
        if (socket->waitForReadyRead()) {
            QString data = socket->readAll();
            const QStringList lines = data.split("\n");
            if (lines.size() > 0) {
                const QUrlQuery result = QUrlQuery(QUrl(lines[0].split(" ")[1]));
                token = result.queryItemValue("code");
                if (state == result.queryItemValue("state")) {
                    _core->request()->loginWithOAuth(token);
}
            }
        }

        const QString body("<html><head><meta http-equiv='refresh' content='0;url=https://poe.rory.io/adamant/finished'></head><body>Please return to Adamant.</body></html>");

        auto response = body.toUtf8();
        socket->write("HTTP/1.1 200 OK\n");
        socket->write("Content-Type: text/html; charset=utf-8\n");
        socket->write(QString("Content-Length: %1\n").arg(response.size()).toUtf8());
        socket->write("\n");
        socket->write(response);
        socket->close();
    });
}

LoginOAuthDialog::~LoginOAuthDialog() {
    _server->close();
    delete ui;
}

void LoginOAuthDialog::showError(const QString &error) {
    ui->errorLabel->show();
    ui->errorLabel->setText(error);
    setEnabled(true);
}

int LoginOAuthDialog::exec() {
    setEnabled(true);

    QString uuid = QUuid::createUuid().toString();
    state = uuid.replace("{", "").replace("}", "").replace("-", "");

    QUrl redirect("http://localhost");
    redirect.setPort(_server->serverPort());

    QUrl url = Session::OAuthAuthorizeUrl();
    QUrlQuery query;
    query.addQueryItem("client_id", "adamant");
    query.addQueryItem("response_type", "code");
    query.addQueryItem("state", state);
    query.addQueryItem("redirect_uri", redirect.toString());
    url.setQuery(query);
    QDesktopServices::openUrl(url);

    return QDialog::exec();
}
