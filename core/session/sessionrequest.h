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
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender()); \
    if (reply->error() != QNetworkReply::NoError) { \
        _session->logError(QString("Network error in %1: %2").arg(__FUNCTION__).arg(reply->errorString())); \
        reply->deleteLater(); \
        return; \
    }

class CORE_EXTERN Session::Request : public QObject
{
    Q_OBJECT
public:
    Request(Session* parent, QNetworkAccessManager* manager);

public slots:
    void loginWithSessionId(const QString &sessionId);
    void loginWithOAuth(const QString &authorizationCode);

    void fetchProfileData();
    void fetchAccountBadge(const QString &badge, const QString &url);
    void fetchImage(const QString &url, const QVariant &variant = QVariant());

    void fetchAccountStashTabs(const QString &accountName, const QString &league, int tabIndex = 0,
                                           bool tabs = true, QVariant data = QVariant());
    void fetchAccountCharacters(const QString &accountName,
                                            QVariant data = QVariant());
    void fetchAccountCharacterItems(const QString &accountName, const QString &character,
                                                QVariant data = QVariant());

    void fetchLeagues();

    void setTimeout(int timeout);
private slots:
    void onOAuthResultPath();
    void onLoginPageResult();
    void onAccountPageResult();
    void onAccountStashTabsResult();
    void onAccountCharactersResult();
    void onAccountCharacterItemsResult();
    void onLeaguesResult();

    void onImageResult(const QString &path, const QImage &image, const QVariant &data);

    void onProfileData();
protected:
    static const QString getCSRFToken(const QByteArray& data);
    static const QString getAccountAvatar(const QByteArray &data);
    static const QString getAccountName(const QByteArray &data);
    static int getAccountMessagesUnread(const QByteArray &data);
    static const QMap<QString,QString> getAccountBadges(const QByteArray &data);

signals:
    void loginResult(int result, QString resultString);
    void profileData(QString data);
    void profileAvatarImage(QImage image);
    void profileBadgeImage(QString badge, QImage image);
    void sessionIdChanged(QString newSessionId);

    void accountStashTabs(QString league, QByteArray result, QVariant data);
    void accountStashTabsJson(QString league, QJsonDocument doc, QVariant data);

    void accountCharacters(QByteArray result, QVariant data);
    void accountCharactersJson(QJsonDocument doc, QVariant data);

    void accountCharacterItems(QString character, QByteArray result, QVariant data);
    void accountCharacterItemsJson(QString character, QJsonDocument doc, QVariant data);

    void leaguesList(QStringList leagues);
private:
    const Session* _session;
    QNetworkAccessManager* _manager;

    ImageCache* _cache;
};
Q_DECLARE_METATYPE(Session::Request*)

#endif // SESSIONREQUEST_H
