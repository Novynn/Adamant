#include "oauthhandler.h"
#include "session/session.h"

#include <QDesktopServices>
#include <QUrlQuery>
#include <QUuid>

OAuthHandler::OAuthHandler()
    : QObject()
    , _server(QSharedPointer<QTcpServer>::create(this))
    , _sensitiveSettings("sensitive.ini", QSettings::IniFormat) {

    // NOTE(rory): Use the access token we already have
    _sensitiveSettings.beginGroup("session");
    _token = _sensitiveSettings.value("access_token").toString();
    _sensitiveSettings.endGroup();

    connect(_server.data(), &QTcpServer::newConnection, this, [this](){
        auto socket = _server->nextPendingConnection();
        socket->waitForConnected();
        if (socket->waitForReadyRead()) {
            QString data = socket->readAll();
            const QStringList lines = data.split("\n");
            if (lines.size() > 0) {
                const QUrlQuery result = QUrlQuery(QUrl(lines[0].split(" ")[1]));
                _code = result.queryItemValue("code");
                if (state == result.queryItemValue("state")) {
                    emit oauthCodeAcquired(_code);
                    _server->close();
                }
            }
        }

        const QString body("<html><head><script type='text/javascript'>window.top.close();</script><meta http-equiv='refresh' content='0;url=https://poe.rory.io/adamant/finished'></head><body>Please return to Adamant.</body></html>");

        auto response = body.toUtf8();
        socket->write("HTTP/1.1 200 OK\n");
        socket->write("Content-Type: text/html; charset=utf-8\n");
        socket->write(QString("Content-Length: %1\n").arg(response.size()).toUtf8());
        socket->write("\n");
        socket->write(response);
        socket->close();
    });
}

bool OAuthHandler::listen() {
    if (_server->isListening())
        return true;

    if (!_server->listen(QHostAddress::LocalHost))
        return false;

    return true;
}

void OAuthHandler::signIn(bool force) {
    if (!force) {
        if (hasAccessToken()) {
            emit oauthAccessTokenAcquired(getAccessToken());
            return;
        }
    }

    const QByteArray uuid = QUuid::createUuid().toRfc4122();
    state = uuid.toHex();

    QUrl redirect("http://localhost");
    redirect.setPort(_server->serverPort());

    QUrl url = Session::OAuthAuthorizeUrl();
    QUrlQuery query;
    query.addQueryItem("client_id", "adamant");
    query.addQueryItem("response_type", "code");
    query.addQueryItem("scope", "profile stashes characters");
    query.addQueryItem("state", state);
    query.addQueryItem("redirect_uri", redirect.toString());
    url.setQuery(query);
    QDesktopServices::openUrl(url);
}

void OAuthHandler::save(const QString &accessToken) {
    _sensitiveSettings.beginGroup("session");
    _sensitiveSettings.setValue("access_token", accessToken);
    _sensitiveSettings.endGroup();
    _sensitiveSettings.sync();

    _token = accessToken;
}
