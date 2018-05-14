#ifndef SESSIONREQUEST_H
#define SESSIONREQUEST_H

#include <core_global.h>
#include "imagecache.h"
#include "session.h"
#include <QNetworkReply>
#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

class CoreService;

#define CHECK_REPLY \
    const int code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(); \
    if (reply->error() != QNetworkReply::NoError) { \
        session->logError(QString("Network error in %1: %2").arg(__FUNCTION__).arg(reply->errorString())); \
        return; \
    }

class CORE_EXTERN Session::Request
{
    Q_GADGET
public:
    enum RequestMethod {
        GET,
        POST,
    };

    Request(QNetworkRequest request, RequestMethod method, const QByteArray &data, Session::RequestResultFunc callback = nullptr);
    Request(QNetworkRequest request, RequestMethod method, Session::RequestResultFunc callback = nullptr)
        : Request(request, method, QByteArray(), callback) {}
    Request(QNetworkRequest request, Session::RequestResultFunc callback = nullptr)
        : Request(request, GET, QByteArray(), callback) {}

    static Session::Request* LoginWithOAuth(Session *session, const QString &authorizationCode, Session::RequestResultFunc callback = nullptr);
    static Session::Request* FetchLeagues(Session* session, Session::LeaguesRequestResultFunc callback = nullptr);
    static Session::Request* FetchCharacters(Session *session, Session::BasicJsonRequestResultFunc callback = nullptr);
    static Session::Request* FetchCharacterItems(Session *session, const QString &character, Session::BasicJsonRequestResultFunc callback = nullptr);
    static Session::Request* FetchStashTabs(Session *session, const QString &league, int tabIndex, bool tabs, Session::BasicJsonRequestResultFunc callback = nullptr);

    inline void setReply(QNetworkReply* reply) {
        _reply = reply;
    }

    inline void setAttribute(AttributeData attr, const QVariant &data) {
        _request->setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + attr), data);
    }

    inline static QVariant GetRequestAttribute(QNetworkRequest request, AttributeData attr) {
        return request.attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + attr));
    }

    inline QVariant getAttribute(AttributeData attr) {
        return GetRequestAttribute(*_request, attr);
    }
public slots:
    void setTimeoutFromNow(int timeout);
protected:
private:
    RequestMethod _method;
    QByteArray _data;
    QNetworkRequest* _request;
    QNetworkReply* _reply;
    Session::RequestResultFunc _callback;
    quint64 _timeout;
    bool _needsLogin;

    friend class Session;
};
Q_DECLARE_METATYPE(Session::Request*)

#endif // SESSIONREQUEST_H
