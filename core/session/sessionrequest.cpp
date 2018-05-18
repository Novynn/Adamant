#include "sessionrequest.h"
#include "core.h"

#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QUrlQuery>

Session::Request::Request(QNetworkRequest request, RequestMethod method, const QByteArray &data, Session::RequestResultFunc callback)
    : _method(method)
    , _data(data)
    , _request(new QNetworkRequest(request))
    , _reply(nullptr)
    , _callback(callback)
    , _timeout(0)
    , _needsLogin(true) {
}

void Session::Request::setTimeoutFromNow(int timeout) {
    _timeout = QDateTime::currentMSecsSinceEpoch() + timeout;
}

Session::Request* Session::Request::FetchLeagues(Session* session, Session::LeaguesRequestResultFunc callback) {
    QNetworkRequest request = session->createRequest(LeaguesUrl());
    request.setRawHeader("Authorization", QByteArray()); // NOTE(rory): Clear authorization to access as a guest
    request.setAttribute(QNetworkRequest::CookieSaveControlAttribute, QNetworkRequest::Manual);

    auto result = new Session::Request(request, [callback](Session* session, QNetworkReply* reply){
        CHECK_REPLY;

        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);

        QStringList list;
        for (auto league : doc.array()) {
            auto obj = league.toObject();
            list << obj.value("id").toString();
            // TODO(rory): We should really store more than just the league ID here, as we might want
            //             to treat SSF leagues separately sometimes (such as disabling shops)
        }

        if (callback) {
            callback(session, list);
        }
    });
//    result->_needsLogin = false;

    session->enqueueRequest(result);
    return result;
}

Session::Request* Session::Request::FetchCharacters(Session* session, Session::BasicJsonRequestResultFunc callback) {
    QNetworkRequest request = session->createRequest(CharactersUrl());

    auto result = new Session::Request(request, [callback](Session* session, QNetworkReply* reply){
        CHECK_REPLY;

        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);

        if (callback) {
            callback(session, doc);
        }
    });
    session->enqueueRequest(result);
    return result;
}

Session::Request* Session::Request::FetchCharacterItems(Session* session, const QString &character, Session::BasicJsonRequestResultFunc callback) {
    const QString account = session->accountName();

    QUrl url = CharacterItemsUrl();
    QUrlQuery query;
    query.addQueryItem("accountName", account);
    query.addQueryItem("character", character);
    url.setQuery(query);
    QNetworkRequest request = session->createRequest(url);

    auto result = new Session::Request(request, [callback](Session* session, QNetworkReply* reply){
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);

        if (callback) {
            callback(session, doc);
        }
    });
    session->enqueueRequest(result);
    return result;
}

Session::Request* Session::Request::FetchStashTabs(Session* session, const QString &league, int tabIndex, bool tabs, Session::BasicJsonRequestResultFunc callback) {
    const QString account = session->accountName();

    QUrl url = StashUrl();
    QUrlQuery query;
    query.addQueryItem("accountName", account);
    query.addQueryItem("league", league);
    query.addQueryItem("tabs", QString::number(tabs ? 1 : 0));
    query.addQueryItem("tabIndex", QString::number(tabIndex));
    url.setQuery(query);
    QNetworkRequest request = session->createRequest(url);

    auto result = new Session::Request(request, [callback](Session* session, QNetworkReply* reply){
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);

        if (callback) {
            callback(session, doc);
        }
    });
    session->enqueueRequest(result);
    return result;
}
