#include "sessionrequest.h"
#include "core.h"

#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QUrlQuery>
#include <QRegularExpression>
#include <QInputDialog>

Session::Request::Request(Session *parent, QNetworkAccessManager* manager)
    : QObject(parent)
    , _session(parent)
    , _manager(manager)
    , _cache(new ImageCache(this))
{
    connect(_cache, &ImageCache::onImage, this, &Session::Request::Request::onImageResult);
    connect(_manager, &QNetworkAccessManager::sslErrors, this, [this] (QNetworkReply* reply, const QList<QSslError> &errors) {
        Q_UNUSED(errors);
        reply->ignoreSslErrors();
    });

    connect(_manager,  &QNetworkAccessManager::networkAccessibleChanged, this, [this](QNetworkAccessManager::NetworkAccessibility accessible) {
        qDebug() << "Network accessibility changed: " << accessible;
    });
#if 0
        QNetworkCookie debugCookie("XDEBUG_SESSION", "XDEBUG_VSCODE");
        debugCookie.setPath("/");
        debugCookie.setDomain(Session::CookieDomain());
        _manager->cookieJar()->insertCookie(debugCookie);
#endif
}

void Session::Request::setTimeout(int timeout) {
    Q_UNUSED(timeout)
}

void Session::Request::loginWithOAuth(const QString &authorizationCode) {
    QUrl url = Session::OAuthTokenUrl();
    QUrlQuery query;
    query.addQueryItem("client_id", Session::OAuthClientId());
    query.addQueryItem("code", authorizationCode);
    query.addQueryItem("grant_type", "authorization_code");
    QNetworkRequest request = _session->createRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply *r = _manager->post(request, query.toString().toUtf8());
    connect(r, &QNetworkReply::finished, this, &Session::Request::Request::onOAuthResultPath);
}

void Session::Request::loginWithOAuthAccessToken(const QString &accessToken) {
    // TODO(rory): Verify this somehow?
    emit loginResult(0x00, accessToken);
    fetchProfileData();
}

void Session::Request::onOAuthResultPath() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
    if (reply->error()) {
        qDebug() << __FUNCTION__ << reply->errorString();
        qDebug() << __FUNCTION__ << reply->readAll();
        emit loginResult(0x01, "A network error occured: " + reply->errorString());
    }
    else {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QString token = doc.object().value("access_token").toString();

        emit loginResult(0x00, token);
    }
    reply->deleteLater();
}

void Session::Request::fetchProfileData() {
    QNetworkRequest request = _session->createRequest(ProfileDataUrl());
    QNetworkReply *r = _manager->get(request);
    connect(r, &QNetworkReply::finished, this, &Session::Request::Request::onProfileData);
}

void Session::Request::fetchAccountBadge(const QString &badge, const QString &url) {
    QVariantHash data = {
        {"type", "badge"},
        {"badge", badge}
    };

    fetchImage(url, data);
}

void Session::Request::fetchImage(const QString &url, const QVariant &variant) {
    QImage image = _cache->getImage(url, variant);
    if (!image.isNull()) {
        onImageResult(url, image, variant);
    }
}

void Session::Request::fetchAccountStashTabs(const QString &accountName, const QString &league, int tabIndex,
                                     bool tabs, QVariant data) {
    QUrlQuery query;
    query.addQueryItem("accountName", accountName);
    query.addQueryItem("league", league);
    query.addQueryItem("tabIndex", QString::number(tabIndex));
    query.addQueryItem("tabs", tabs ? "1" : "0");
    QUrl url = StashUrl();
    url.setQuery(query);

    QNetworkRequest request = _session->createRequest(url);
    Session::SetCustomRequestAttribute(&request, UserData, data);
    Session::SetCustomRequestAttribute(&request, League, league);
    QNetworkReply *r = _manager->get(request);

    connect(r, &QNetworkReply::finished, this, &Session::Request::Request::onAccountStashTabsResult);
}

void Session::Request::fetchAccountCharacters(const QString &accountName,
                                      QVariant data) {
    QUrlQuery query;
    query.addQueryItem("accountName", accountName);
    QUrl url = CharactersUrl();
    url.setQuery(query);

    QNetworkRequest request = _session->createRequest(url);
    Session::SetCustomRequestAttribute(&request, UserData, data);
    QNetworkReply *r = _manager->get(request);

    connect(r, &QNetworkReply::finished, this, &Session::Request::Request::onAccountCharactersResult);
}

void Session::Request::fetchAccountCharacterItems(const QString &accountName, const QString &character,
                                          QVariant data) {
    QUrlQuery query;
    query.addQueryItem("accountName", accountName);
    query.addQueryItem("character", character);
    QUrl url = CharacterItemsUrl();
    url.setQuery(query);

    QNetworkRequest request = _session->createRequest(url);
    Session::SetCustomRequestAttribute(&request, UserData, data);
    Session::SetCustomRequestAttribute(&request, Character, character);
    QNetworkReply *r = _manager->get(request);

    connect(r, &QNetworkReply::finished, this, &Session::Request::Request::onAccountCharacterItemsResult);
}

void Session::Request::fetchLeagues() {
    QNetworkRequest request = _session->createRequest(LeaguesUrl());
    request.setAttribute(QNetworkRequest::CookieSaveControlAttribute, QNetworkRequest::Manual);
    QNetworkReply *r = _manager->get(request);

    connect(r, &QNetworkReply::finished, this, &Session::Request::Request::onLeaguesResult);
}

void Session::Request::onProfileData() {
    CHECK_REPLY;

    QByteArray response = reply->readAll();
    if (response.isEmpty() || code != 200) {
        emit loginResult(0x01, "Failed to fetch profile data");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(response);

    const QString avatar = Session::FixRelativeUrl(doc.object().value("avatar_url").toString());
    // Request Avatar
    if (!avatar.isEmpty()) {
        if (!_avatars.contains(avatar))
            _avatars.append(avatar);
        fetchImage(avatar);
    }

    const QJsonObject badges = doc.object().value("badges").toObject();
    // Request Badges
    for (const QString &key : badges.keys()){
        const QString url = badges.value(key).toString();
        if (url.isEmpty()) continue;
        fetchAccountBadge(key, url);
    }
    reply->deleteLater();

    emit profileData(doc.toJson());
}

void Session::Request::onLeaguesResult() {
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

    emit leaguesList(list);

    reply->deleteLater();
}

void Session::Request::onImageResult(const QString &path, const QImage &image, const QVariant &data) {
    Q_UNUSED(path);
    QVariantHash hash = data.toHash();
    const QString type = hash.value("type").toString();
    if (type == "badge") {
        const QString badge = hash.value("badge").toString();
        emit profileBadgeImage(badge, image);
    }
    else if (type == "avatar") {
        emit profileAvatarImage(image);
    }
    else {
        qWarning() << "Unknown image type recieved in " << __FUNCTION__;
    }
}

void Session::Request::onAccountStashTabsResult() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
    QByteArray response;
    if (reply->error()) {
        const QString errorMessage = QString("Network error in %1: %2").arg(__FUNCTION__).arg(reply->errorString());
        _session->logError(errorMessage);
        QJsonObject error({
            {
                "error", QJsonObject({
                    {"message", errorMessage},
                    {"internal", true}
                })
            }
        });
        response = QJsonDocument(error).toJson();
    }
    else {
        response = reply->readAll();
    }

    QNetworkRequest request = reply->request();
    QVariant data = Session::GetCustomRequestAttribute(&request, UserData);
    QString league = Session::GetCustomRequestAttribute(&request, League).toString();

    emit accountStashTabs(league, response, data);
    QJsonDocument doc = QJsonDocument::fromJson(response);
    emit accountStashTabsJson(league, doc, data);

    reply->deleteLater();
}

void Session::Request::onAccountCharactersResult() {
    CHECK_REPLY;

    QByteArray response = reply->readAll();
    QNetworkRequest request = reply->request();
    QVariant data = Session::GetCustomRequestAttribute(&request, UserData);

    emit accountCharacters(response, data);
    QJsonDocument doc = QJsonDocument::fromJson(response);
    emit accountCharactersJson(doc, data);

    reply->deleteLater();
}

void Session::Request::onAccountCharacterItemsResult() {
    CHECK_REPLY;

    QByteArray response = reply->readAll();
    QNetworkRequest request = reply->request();
    QVariant data = Session::GetCustomRequestAttribute(&request, UserData);
    QString character = Session::GetCustomRequestAttribute(&request, Character).toString();

    emit accountCharacterItems(character, response, data);
    QJsonDocument doc = QJsonDocument::fromJson(response);
    emit accountCharacterItemsJson(character, doc, data);

    reply->deleteLater();
}

const QString Session::Request::getCSRFToken(const QByteArray &data) {
    QRegularExpression expr("<input.+?name=\\\"hash\\\" value=\\\"(?<hash>.+?)\\\".+?id=\\\"hash\\\">");
    QRegularExpressionMatch match = expr.match(data);
    if (match.isValid() && match.hasMatch()) {
        return match.captured("hash");
    }
    return QString();
}

const QString Session::Request::getAccountAvatar(const QByteArray &data) {
    QRegularExpression expr("<img\\s+src=\\\"(?<image>.+?)\\\"\\s+alt=\\\"Avatar\\\"");
    QRegularExpressionMatch match = expr.match(data);
    if (match.isValid() && match.hasMatch()) {
        const QString image = match.captured("image");
        return Session::FixRelativeUrl(image);
    }
    return QString();
}

const QString Session::Request::getAccountName(const QByteArray &data) {
    QRegularExpression expr("<h1\\s+class=\\\"name\\\">(?<name>.+?)<\\/h1>");
    QRegularExpressionMatch match = expr.match(data);
    if (match.isValid() && match.hasMatch()) {
        return match.captured("name");
    }
    return QString();
}

int Session::Request::getAccountMessagesUnread(const QByteArray &data) {
    QRegularExpression expr("<a class=\\\"statusItem privateMessageStatus (?<has>hasMessages)?\\\" href=\\\"(?<url>.+?)\\\">(?<text>.+?)<\\/a>");
    QRegularExpressionMatch match = expr.match(data);
    if (match.isValid() && match.hasMatch()) {
        if (match.captured("has").isEmpty()){
            return 0;
        }
        else {
            QString text = match.captured("text");
            int index = text.indexOf(" ");
            if (index == -1) {
                return 0;
            }

            // Will return 0 if invalid, which is fine.
            return text.left(index).toInt();
        }
    }
    return 0;

}

const QMap<QString, QString> Session::Request::getAccountBadges(const QByteArray &data) {
    QRegularExpression expr("<div class=\\\"roleLabel.+?\\\"><img src=\\\"(?<url>.+?)\\\" alt=\\\"(?<name>.+?)\\\" \\/><\\/div>");
    QRegularExpressionMatchIterator iter = expr.globalMatch(data);
    QMap<QString,QString> result;
    while (iter.hasNext()) {
        QRegularExpressionMatch match = iter.next();
        if (match.isValid() && match.hasMatch()) {
            QString url = match.captured("url");
            if (url.startsWith("/")) url.prepend("https://web.poecdn.com");
            result.insert(match.captured("name"), url);
        }
    }

    //        result.insert("Exalted", "https://web.poecdn.com/image/forum/supporter-tag/open-beta/exalted.png?v=2");
    //        result.insert("Champion", "https://web.poecdn.com/image/forum/supporter-tag/release/champion.png?v=4");
    //        result.insert("Grandmaster", "https://web.poecdn.com/image/forum/supporter-tag/release2/Grandmaster.png");
    //        result.insert("Highgate", "https://web.poecdn.com/image/forum/supporter-tag/awakening/Highgate.png");
    //        result.insert("Ascendant", "https://web.poecdn.com/image/forum/supporter-tag/ascendancy/Ascendant.png");

    return result;
}


