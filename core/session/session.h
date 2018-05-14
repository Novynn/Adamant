#ifndef SESSION_H
#define SESSION_H

#include <core_global.h>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QScopedPointer>
#include <QPointer>
#include <QQueue>
#include <QUuid>

class CoreService;

class CORE_EXTERN Session : public QObject
{
    Q_OBJECT
public:
    typedef std::function<void(Session*, QNetworkReply*)> RequestResultFunc;
    typedef std::function<void(Session*, const QStringList &)> LeaguesRequestResultFunc;
    typedef std::function<void(Session*, const QJsonDocument &)> BasicJsonRequestResultFunc;
    class Request;
    class ForumRequest;

    enum AttributeData {
        RequestId = 0x01,
        LoginSessionId,
        Badge,
        UserData,
        League,
        Character,
        ForumSubmissionData
    };

    enum SessionLoginState {
        Idle,
        Success,
        Failed
    };

    static QUrl BaseUrl() {
        return QUrl("https://www.pathofexile.com");
    }

    static QUrl APIUrl() {
        return QUrl("https://www.pathofexile.com/api");
    }

    static QUrl OAuthAuthorizeUrl() {
        return QUrl(BaseUrl().toString() + "/oauth/authorize");
    }

    static QUrl OAuthTokenUrl() {
        return QUrl(BaseUrl().toString() + "/oauth/token");
    }

    static QUrl LoginUrl() {
        return QUrl(BaseUrl().toString() + "/login");
    }

    static QUrl MainUrl() {
        return QUrl(BaseUrl().toString() + "/news");
    }

    static QUrl ProfileDataUrl() {
        return QUrl(APIUrl().toString() + "/profile");
    }

    static QUrl AccountUrl() {
        return QUrl(BaseUrl().toString() + "/my-account");
    }

    static QUrl StashUrl() {
        return QUrl(BaseUrl().toString() + "/character-window/get-stash-items");
    }

    static QUrl CharacterItemsUrl() {
        return QUrl(BaseUrl().toString() + "/character-window/get-items");
    }

    static QUrl CharactersUrl() {
        return QUrl(BaseUrl().toString() + "/character-window/get-characters");
    }

    static QUrl LeaguesUrl() {
        return QUrl(BaseUrl().toString() + "/api/leagues?type=main");
    }

    static QString SessionIdCookie() {
        return "POESESSID";
    }

    Session(CoreService* parent);
    ~Session();
    static QString CookieDomain() {
        return ".pathofexile.com";
    }

    static QString OAuthClientId() {
        return "adamant";
    }

    static QString FixRelativeUrl(const QString &url) {
        return url.startsWith("/") ? Session::BaseUrl().toString() + url : url;
    }

    void resetLoginState() {
        _loginState = SessionLoginState::Idle;
    }

    SessionLoginState loginState() const {
        return _loginState;
    }

    Q_INVOKABLE const QString accessToken() const {
        return _accessToken;
    }

    Q_INVOKABLE const QString accountName() const {
        return _accountName;
    }

    Q_INVOKABLE const QStringList leagues() const {
        return _leagues;
    }

    void logError(const QString &error) const;

    static Session::Request* LoginWithOAuth(Session *session, const QString &authorizationCode);
    static Session::Request *LoginWithOAuthAccessToken(Session *session, const QString &accessToken);
    static Session::Request* UpdateProfile(Session *session);

    static void SetCustomRequestAttribute(QNetworkRequest* request, AttributeData attr, const QVariant &data);
    static const QVariant GetCustomRequestAttribute(const QNetworkRequest *request, AttributeData attr);
    QNetworkRequest createRequest(const QUrl &url) const;
    bool enqueueRequest(Request *request);
    bool processRequestQueue();

    void updateLeagues(const QStringList &leagues);
    void updateAccessToken(const QString &token);
signals:
    void sessionChange();
    void loginStateChange(Session::SessionLoginState state);
protected:
    void updateLoginState(SessionLoginState state);
private:
    CoreService* _core;
    SessionLoginState _loginState;
    QString _accessToken;
    QString _accountName;
    QStringList _leagues;
    QQueue<Session::Request*> _queue;
    QMap<QUuid, Session::Request*> _responses;

    QScopedPointer<QNetworkAccessManager> _manager;
};

#endif // SESSION_H
