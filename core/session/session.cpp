#include "session.h"
#include "core.h"
#include "sessionrequest.h"
#include "forum/forumrequest.h"

Session::Session(CoreService *parent)
    : QObject(parent)
    , _core(parent)
    , _loginState (SessionLoginState::Idle) {
    _manager.reset(new QNetworkAccessManager(parent));
    _request.reset(new Request(this, _manager.data()));
    _forumRequest.reset(new ForumRequest(this, _manager.data()));

    connect(_request.data(), &Session::Request::loginResult, this, [this](int result, const QString &data){
        if (result == 0x00) {
            _accessToken = data;
            _loginState = SessionLoginState::Success;
        }
        else {
            _loginState = SessionLoginState::Failed;
        }
        emit sessionChange();
    });

    connect(_request.data(), &Session::Request::profileData, this, [this](QString data) {
        QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8());
        _accountName = doc.object().value("name").toString();
        emit sessionChange();
    });

    connect(_request.data(), &Session::Request::leaguesList, this, [this](QStringList leagues) {
        _leagues = leagues;
        emit sessionChange();
    });
}

Session::~Session() {

}

void Session::logError(const QString &error) const {
    emit _core->message(error, QtMsgType::QtWarningMsg);
}

void Session::SetCustomRequestAttribute(QNetworkRequest *request, Session::AttributeData attr, const QVariant &data) {
    request->setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + attr), data);
}

const QVariant Session::GetCustomRequestAttribute(const QNetworkRequest *request, Session::AttributeData attr) {
    return request->attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + attr));
}

QNetworkRequest Session::createRequest(const QUrl &url) const {
    auto request = QNetworkRequest(url);
    const QString token = accessToken();
    if (!token.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());
    }
    return request;
}
