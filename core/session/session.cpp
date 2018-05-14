#include "session.h"
#include "core.h"
#include "sessionrequest.h"
#include "forum/forumrequest.h"

#include <QUrlQuery>
#include <QUuid>

Session::Session(CoreService *parent)
    : QObject(parent)
    , _core(parent)
    , _loginState (SessionLoginState::Idle) {
    _manager.reset(new QNetworkAccessManager(parent));

    connect(_manager.data(), &QNetworkAccessManager::finished, this, [this](QNetworkReply* reply) {
        auto request = reply->request();
        auto key = GetCustomRequestAttribute(&request, RequestId).toUuid();
        auto sessionRequest = _responses.take(key);
        if (sessionRequest) {
            if (sessionRequest->_callback) {
                sessionRequest->_callback(this, reply);
            }
            reply->deleteLater();
        }
    });
}

Session::~Session() {

}

void Session::logError(const QString &error) const {
    qWarning() << error;
}

Session::Request* Session::LoginWithOAuthAccessToken(Session* session, const QString &accessToken) {
    // NOTE(rory): Ideally pathofexile.com would expose a way to verify access tokens
    //             For now we just have to assume it's okay

    session->resetLoginState();
    session->updateAccessToken(accessToken);
    Session::UpdateProfile(session);
}

Session::Request* Session::LoginWithOAuth(Session* session, const QString &authorizationCode) {
    QUrl url = Session::OAuthTokenUrl();
    QUrlQuery query;
    query.addQueryItem("client_id", Session::OAuthClientId());
    query.addQueryItem("code", authorizationCode);
    query.addQueryItem("grant_type", "authorization_code");
    QNetworkRequest request = session->createRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    auto result = new Request(request, Request::POST, query.toString().toUtf8(), [](Session* session, QNetworkReply* reply) {
        QJsonDocument doc;
        if (reply->header(QNetworkRequest::ContentTypeHeader).toString() == "application/json") {
            doc = QJsonDocument::fromJson(reply->readAll());
        }

        const int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (code != 200) {
            qDebug() << code << doc.toJson(QJsonDocument::Indented);
            return;
        }

        const QString token = doc.object().value("access_token").toString();

        session->updateAccessToken(token);
        Session::UpdateProfile(session);
    });

    // NOTE(rory): Nice hacks
    result->_needsLogin = false;

    session->enqueueRequest(result);
    return result;
}

Session::Request* Session::UpdateProfile(Session* session) {
    QNetworkRequest request = session->createRequest(Session::ProfileDataUrl());

    auto result = new Request(request, [](Session* session, QNetworkReply* reply) {
        QJsonDocument doc;
        if (reply->header(QNetworkRequest::ContentTypeHeader).toString() == "application/json") {
            doc = QJsonDocument::fromJson(reply->readAll());
        }

        const int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (code != 200) {
            session->updateLoginState(Failed);
            return;
        }

        const QString name = doc.object().value("name").toString();
        session->_accountName = name;
        emit session->sessionChange();

        if (session->loginState() != Success) {
            session->updateLoginState(Success);
        }
    });

    // NOTE(rory): Nice hacks
    result->_needsLogin = false;

    session->enqueueRequest(result);
    return result;
}

void Session::SetCustomRequestAttribute(QNetworkRequest *request, Session::AttributeData attr, const QVariant &data) {
    request->setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + attr), data);
}

const QVariant Session::GetCustomRequestAttribute(const QNetworkRequest *request, Session::AttributeData attr) {
    return request->attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + attr));
}

QNetworkRequest Session::createRequest(const QUrl &url) const {
    auto request = QNetworkRequest(url);
    // NOTE(rory): pathofexile.com tends to save a bunch of cookies, ignore them
    request.setAttribute(QNetworkRequest::CookieSaveControlAttribute, QNetworkRequest::Manual);

    const QString token = accessToken();
    if (!token.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());
    }
    return request;
}

bool Session::enqueueRequest(Session::Request* request) {
    _queue.enqueue(request);
    return processRequestQueue();
}

bool Session::processRequestQueue() {
    QQueue<Session::Request*> backlog;

    while (!_queue.isEmpty()) {
        auto request = _queue.dequeue();

        if (loginState() != Success && request->_needsLogin) {
            backlog.enqueue(request);
            continue;
        }

        auto key = QUuid::createUuid();
        request->setAttribute(RequestId, key);

        _responses.insert(key, request);
        QByteArray verb;
        switch (request->_method) {
            case Session::Request::GET:     verb = "GET";   break;
            case Session::Request::POST:    verb = "POST";  break;
        }
        auto reply = _manager->sendCustomRequest(*request->_request, verb, request->_data);
        qDebug() << "Request:" << request->_request->url().toString() << key.toString();
        request->setReply(reply);
    }

    _queue = backlog;

    return true;
}

void Session::updateLoginState(SessionLoginState state) {
    _loginState = state;
    emit loginStateChange(state);
    processRequestQueue();
}

void Session::updateLeagues(const QStringList &leagues) {
    _leagues = leagues;
    emit sessionChange();
}

void Session::updateAccessToken(const QString &token) {
    _accessToken = token;
    emit sessionChange();
}
