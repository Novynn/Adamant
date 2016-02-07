#include "sessionrequest.h"

#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QUrlQuery>
#include <QRegularExpression>

Session::Request::Request(QObject *parent)
    : QObject(parent)
    , _manager(new QNetworkAccessManager(this))
    , _cache(new ImageCache(this))
    , _accountName()
    , _sessionId()
{
    connect(_cache, &ImageCache::onImage, this, &Session::Request::Request::onImageResult);
}

void Session::Request::setTimeout(int timeout) {
    Q_UNUSED(timeout)
}

void Session::Request::login(const QString &username, const QString &password) {
    QNetworkRequest request(LoginUrl());
    // TODO(rory): Encode special characters in username / password
    setAttribute(&request, LoginUsername, username);
    setAttribute(&request, LoginPassword, password);

    QNetworkReply *r = _manager->get(request);

    connect(r, &QNetworkReply::finished, this, &Session::Request::Request::onLoginPage);
}

void Session::Request::loginWithSessionId(const QString &sessionId) {
    QNetworkCookie poeCookie(SessionIdCookie().toUtf8(), sessionId.toUtf8());
    poeCookie.setPath("/");
    poeCookie.setDomain(".pathofexile.com");

    _manager->cookieJar()->insertCookie(poeCookie);

    QNetworkRequest request = QNetworkRequest(LoginUrl());
    QNetworkReply *r = _manager->get(request);

    connect(r, &QNetworkReply::finished, this, &Session::Request::Request::onLoginPageResult);
}

void Session::Request::clear() {
    _sessionId.clear();
}

void Session::Request::fetchAccountBadge(const QString &badge, const QString &url) {
    const QString badgeFile = _cache->generateFileName(url);
    if (!_badges.contains(badge))
        _badges.insert(badgeFile, badge);
    fetchImage(url);
}

void Session::Request::fetchImage(const QString &url) {
    QImage image = _cache->getImage(url);
    if (!image.isNull()) {
        onImageResult(url, image);
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

    QNetworkRequest request = QNetworkRequest(url);
    setAttribute(&request, UserData, data);
    setAttribute(&request, League, league);
    QNetworkReply *r = _manager->get(request);

    connect(r, &QNetworkReply::finished, this, &Session::Request::Request::onAccountStashTabsResult);
}

void Session::Request::fetchAccountCharacters(const QString &accountName,
                                      QVariant data) {
    QUrlQuery query;
    query.addQueryItem("accountName", accountName);
    QUrl url = CharactersUrl();
    url.setQuery(query);

    QNetworkRequest request = QNetworkRequest(url);
    setAttribute(&request, UserData, data);
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

    QNetworkRequest request = QNetworkRequest(url);
    setAttribute(&request, UserData, data);
    QNetworkReply *r = _manager->get(request);

    connect(r, &QNetworkReply::finished, this, &Session::Request::Request::onAccountCharacterItemsResult);
}

void Session::Request::fetchLeagues() {
    QNetworkRequest request = QNetworkRequest(LeaguesUrl());
    QNetworkReply *r = _manager->get(request);

    connect(r, &QNetworkReply::finished, this, &Session::Request::Request::onLeaguesResult);
}

void Session::Request::onLoginPage() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
    if (reply->error()) {
        emit loginResult(0x01, "A network error occured: " + reply->errorString());
    }
    else {
        QNetworkRequest request = reply->request();
        // Extract CSF
        const QString hash = getCSRFToken(reply->readAll());
        const QString username = getAttribute(&request, LoginUsername).toString();
        const QString password = getAttribute(&request, LoginPassword).toString();

        QUrlQuery query;
        query.addQueryItem("login_email", username);
        query.addQueryItem("login_password", password);
        query.addQueryItem("hash", hash);
        query.addQueryItem("login", "Login");
        QByteArray data(query.query().toUtf8());

        request = QNetworkRequest(LoginUrl());
        setAttribute(&request, LoginUsername, username);
        setAttribute(&request, LoginPassword, password);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

        QNetworkReply *r = _manager->post(request, data);

        connect(r, &QNetworkReply::finished, this, &Session::Request::Request::onLoginPageResult);
    }
    reply->deleteLater();
}

void Session::Request::onLoginPageResult() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status != 302) {
        emit loginResult(0x01, "Failed to log in (invalid password or expired session ID).");
    }
    else {
        QList<QNetworkCookie> cookies = reply->manager()->cookieJar()->cookiesForUrl(MainUrl());
        for (auto &cookie : cookies) {
            if (QString(cookie.name()) == SessionIdCookie()) {
                _sessionId = cookie.value();
                emit loginResult(0x00, "Logged in!");
                break;
            }
        }

        QNetworkRequest request = QNetworkRequest(AccountUrl());
        QNetworkReply *r = _manager->get(request);

        connect(r, &QNetworkReply::finished, this, &Session::Request::Request::onAccountPageResult);
    }
    reply->deleteLater();
}

void Session::Request::onAccountPageResult() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
    if (reply->error()) {
        qDebug() << "Network error in " << __FUNCTION__ << ": " << reply->errorString();
    }
    else {
        // Regexp for getting last visited, guild, etc
        // <strong>(?<attr>.+?):<\/strong><br\/>\s+?(<a href=\"(?<url>.+?)\">(?<content1>.+?)<\/a>|\s+(?<content2>[A-Za-z0-9 ]+?)\s+<\/p>)


        const QByteArray data = reply->readAll();
        const QString avatar = getAccountAvatar(data);
        const QString name = getAccountName(data);
        const auto badges = getAccountBadges(data);
        const int messages = getAccountMessagesUnread(data);

        // Store the account name
        _accountName = name;

        QJsonObject object;
        object.insert("name", name);
        object.insert("avatar_url", avatar);
        object.insert("messages", messages);
        object.insert("badges", QJsonArray::fromStringList(badges.keys()));
        QJsonDocument temp(object);
        emit profileData(temp.toJson());

        // Request Avatar
        {
            const QString url = BaseUrl().toString() + avatar;
            const QString avatar = _cache->generateFileName(url);
            if (!_avatars.contains(avatar))
                _avatars.append(avatar);
            fetchImage(url);
        }
        // Request Badges
        for (const QString &key : badges.uniqueKeys()){
            const QString url = badges.value(key);
            if (url.isEmpty()) continue;
            fetchAccountBadge(key, url);
        }
    }
    reply->deleteLater();
}

void Session::Request::onLeaguesResult() {
    CHECK_REPLY;

    QByteArray response = reply->readAll();

    QJsonDocument doc = QJsonDocument::fromJson(response);
    QStringList list = doc.object().value("leagues").toVariant().toStringList();
    emit leaguesList(list);

    reply->deleteLater();
}

void Session::Request::onImageResult(const QString &path, const QImage &image) {
    if (_badges.contains(path)) {
        const QString badge = _badges.value(path);
        emit profileBadgeImage(badge, image);
    }
    else if (_avatars.contains(path)) {
        emit profileAvatarImage(image);
    }
}

void Session::Request::onAccountStashTabsResult() {
    CHECK_REPLY;

    QByteArray response = reply->readAll();
    QNetworkRequest request = reply->request();
    QVariant data = getAttribute(&request, UserData);
    QString league = getAttribute(&request, League).toString();

    emit accountStashTabs(league, response, data);
    QJsonDocument doc = QJsonDocument::fromJson(response);
    emit accountStashTabsJson(league, doc, data);

    reply->deleteLater();
}

void Session::Request::onAccountCharactersResult() {
    CHECK_REPLY;

    QByteArray response = reply->readAll();
    QNetworkRequest request = reply->request();
    QVariant data = getAttribute(&request, UserData);

    emit accountCharacters(response, data);
    QJsonDocument doc = QJsonDocument::fromJson(response);
    emit accountCharactersJson(doc, data);

    reply->deleteLater();
}

void Session::Request::onAccountCharacterItemsResult() {
    CHECK_REPLY;

    QByteArray response = reply->readAll();
    QNetworkRequest request = reply->request();
    QVariant data = getAttribute(&request, UserData);
    QString character = getAttribute(&request, Character).toString();

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
        return match.captured("image");
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
            result.insert(match.captured("name"), match.captured("url"));
        }
    }

    //        result.insert("Exalted", "https://p7p4m6s5.ssl.hwcdn.net/image/forum/supporter-tag/open-beta/exalted.png?v=2");
    //        result.insert("Champion", "https://p7p4m6s5.ssl.hwcdn.net/image/forum/supporter-tag/release/champion.png?v=4");
    //        result.insert("Grandmaster", "https://p7p4m6s5.ssl.hwcdn.net/image/forum/supporter-tag/release2/Grandmaster.png");
    //        result.insert("Highgate", "https://p7p4m6s5.ssl.hwcdn.net/image/forum/supporter-tag/awakening/Highgate.png");
    //        result.insert("Ascendant", "https://p7p4m6s5.ssl.hwcdn.net/image/forum/supporter-tag/ascendancy/Ascendant.png");

    return result;
}


