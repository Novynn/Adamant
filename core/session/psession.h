#ifndef PSESSION_H
#define PSESSION_H

#include <core_global.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QObject>
#include <QRegularExpression>
#include <QUrlQuery>
#include <QApplication>
#include <QJsonArray>
#include <QImage>

#define CHECK_REPLY \
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender()); \
    if (reply->error()) { \
        LogError(QString("Network error in %1: %2").arg(__FUNCTION__).arg(reply->errorString())); \
        reply->deleteLater(); \
        return; \
    }

class ImageCache;

class CORE_EXTERN PSession : public QObject
{
    Q_OBJECT
public:
    explicit PSession(QObject *parent = 0, QString accountName = QString(),
                      QString sessionId = QString());

    enum AttributeData {
        LoginUsername = 0x01,
        LoginPassword,
        LoginSessionId,
        Badge,
        UserData,
        League,
        Character
    };

    static QUrl BaseUrl() {
        return QUrl("https://www.pathofexile.com");
    }

    static QUrl LoginUrl() {
        return QUrl(BaseUrl().toString() + "/login");
    }

    static QUrl MainUrl() {
        return QUrl(BaseUrl().toString() + "/news");
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
        return QUrl("http://poe.rory.io/api/v1/leagues");
    }

    static QString SessionIdCookie() {
        return "POESESSID";
    }

    static void LogError(const QString &error) {
        qDebug() << qPrintable(error);
    }
    QString SessionId() const {
        return _sessionId;
    }

    QString AccountName() const {
        return _accountName;
    }

    Q_PROPERTY(QString SessionId MEMBER _sessionId NOTIFY SessionIdChanged READ SessionId)
    Q_PROPERTY(QString AccountName MEMBER _accountName READ AccountName)
public slots:
    void Login(const QString &username, const QString &password);
    void LoginWithSessionId(const QString &sessionId);
    void Clear();
    void GetAccountBadge(const QString &badge, const QString &url);
    void FetchAccountStashTabs(const QString &accountName, const QString &league, int tabIndex = 0,
                                           bool tabs = true, QVariant data = QVariant());
    void FetchAccountCharacters(const QString &accountName,
                                            QVariant data = QVariant());
    void FetchAccountCharacterItems(const QString &accountName, const QString &character,
                                                QVariant data = QVariant());

    void FetchLeagues();

private slots:
    void OnLoginPage();
    void OnLoginPageResult();
    void OnAccountPageResult();
    void OnAccountStashTabsResult();
    void OnAccountCharactersResult();
    void OnAccountCharacterItemsResult();
    void OnLeaguesResult();

protected:
    static const QString GetCSRFToken(const QByteArray& data);
    static const QString GetAccountAvatar(const QByteArray &data);
    static const QString GetAccountName(const QByteArray &data);
    static int GetAccountMessagesUnread(const QByteArray &data);
    static const QMap<QString,QString> GetAccountBadges(const QByteArray &data);

signals:
    void LoginResult(int result, QString resultString);
    void ProfileData(QString data);
    void ProfileAvatarImage(QImage image);
    void ProfileBadgeImage(QString badge, QImage image);
    void SessionIdChanged(QString newSessionId);

    void AccountStashTabs(QString league, QByteArray result, QVariant data);
    void AccountStashTabsJson(QString league, QJsonDocument doc, QVariant data);

    void AccountCharacters(QByteArray result, QVariant data);
    void AccountCharactersJson(QJsonDocument doc, QVariant data);

    void AccountCharacterItems(QString character, QByteArray result, QVariant data);
    void AccountCharacterItemsJson(QString character, QJsonDocument doc, QVariant data);

    void LeaguesList(QStringList leagues);
private:
    QString _sessionId;
    QString _accountName;
    QNetworkAccessManager* _manager;
    ImageCache* _cache;

    QHash<QString, QString> _badges;
    QStringList _avatars;

    inline void SetAttribute(QNetworkRequest* request, AttributeData attr, const QVariant &data) {
        request->setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + attr), data);
    }

    inline QVariant GetAttribute(QNetworkRequest* request, AttributeData attr) {
        return request->attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + attr));
    }
};
Q_DECLARE_METATYPE(PSession*)

#endif // PSESSION_H
