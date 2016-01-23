#include "psession.h"
#include "imagecache.h"

PSession::PSession(QObject *parent, QString accountName, QString sessionId)
    : QObject(parent)
    , _manager(new QNetworkAccessManager(this))
    , _cache(new ImageCache(this))
    , _accountName(accountName)
    , _sessionId(sessionId)
{
    connect(_cache, &ImageCache::OnImage, [this](const QString &path, QImage image) {
        if (_badges.contains(path)) {
            const QString badge = _badges.value(path);
            emit ProfileBadgeImage(badge, image);
        }
        else if (_avatars.contains(path)) {
            emit ProfileAvatarImage(image);
        }
    });
}

void PSession::Login(const QString &username, const QString &password) {
    QNetworkRequest request(LoginUrl());
    // TODO(rory): Encode special characters in username / password
    SetAttribute(&request, LoginUsername, username);
    SetAttribute(&request, LoginPassword, password);

    QNetworkReply *r = _manager->get(request);

    connect(r, &QNetworkReply::finished, this, &PSession::OnLoginPage);
}

void PSession::LoginWithSessionId(const QString &sessionId) {
    QNetworkCookie poeCookie(SessionIdCookie().toUtf8(), sessionId.toUtf8());
    poeCookie.setPath("/");
    poeCookie.setDomain(".pathofexile.com");

    _manager->cookieJar()->insertCookie(poeCookie);

    QNetworkRequest request = QNetworkRequest(LoginUrl());
    QNetworkReply *r = _manager->get(request);

    connect(r, &QNetworkReply::finished, this, &PSession::OnLoginPageResult);
}

void PSession::Clear() {
    _sessionId.clear();
}

void PSession::GetAccountBadge(const QString &badge, const QString &url) {
    const QString badgeFile = _cache->GenerateFileName(url);
    _badges.insert(badgeFile, badge);
    _cache->GetImage(url);
}

void PSession::FetchAccountStashTabs(const QString &accountName, const QString &league, int tabIndex,
                                     bool tabs, QVariant data) {
    QUrlQuery query;
    query.addQueryItem("accountName", accountName);
    query.addQueryItem("league", league);
    query.addQueryItem("tabIndex", QString::number(tabIndex));
    query.addQueryItem("tabs", tabs ? "1" : "0");
    QUrl url = StashUrl();
    url.setQuery(query);

    QNetworkRequest request = QNetworkRequest(url);
    SetAttribute(&request, UserData, data);
    SetAttribute(&request, League, league);
    QNetworkReply *r = _manager->get(request);

    connect(r, &QNetworkReply::finished, this, &PSession::OnAccountStashTabsResult);
}

void PSession::FetchAccountCharacters(const QString &accountName,
                                      QVariant data) {
    QUrlQuery query;
    query.addQueryItem("accountName", accountName);
    QUrl url = CharactersUrl();
    url.setQuery(query);

    QNetworkRequest request = QNetworkRequest(url);
    SetAttribute(&request, UserData, data);
    QNetworkReply *r = _manager->get(request);

    connect(r, &QNetworkReply::finished, this, &PSession::OnAccountCharactersResult);
}

void PSession::FetchAccountCharacterItems(const QString &accountName, const QString &character,
                                          QVariant data) {
    QUrlQuery query;
    query.addQueryItem("accountName", accountName);
    query.addQueryItem("character", character);
    QUrl url = CharacterItemsUrl();
    url.setQuery(query);

    QNetworkRequest request = QNetworkRequest(url);
    SetAttribute(&request, UserData, data);
    QNetworkReply *r = _manager->get(request);

    connect(r, &QNetworkReply::finished, this, &PSession::OnAccountCharacterItemsResult);
}

void PSession::FetchLeagues() {
    QNetworkRequest request = QNetworkRequest(LeaguesUrl());
    QNetworkReply *r = _manager->get(request);

    connect(r, &QNetworkReply::finished, this, &PSession::OnLeaguesResult);
}

void PSession::OnLoginPage() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
    if (reply->error()) {
        emit LoginResult(0x01, "A network error occured: " + reply->errorString());
    }
    else {
        QNetworkRequest request = reply->request();
        // Extract CSF
        const QString hash = GetCSRFToken(reply->readAll());
        const QString username = GetAttribute(&request, LoginUsername).toString();
        const QString password = GetAttribute(&request, LoginPassword).toString();

        QUrlQuery query;
        query.addQueryItem("login_email", username);
        query.addQueryItem("login_password", password);
        query.addQueryItem("hash", hash);
        query.addQueryItem("login", "Login");
        QByteArray data(query.query().toUtf8());

        request = QNetworkRequest(LoginUrl());
        SetAttribute(&request, LoginUsername, username);
        SetAttribute(&request, LoginPassword, password);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

        QNetworkReply *r = _manager->post(request, data);

        connect(r, &QNetworkReply::finished, this, &PSession::OnLoginPageResult);
    }
    reply->deleteLater();
}

void PSession::OnLoginPageResult() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status != 302) {
        emit LoginResult(0x01, "Failed to log in (invalid password or expired session ID).");
    }
    else {
        QList<QNetworkCookie> cookies = reply->manager()->cookieJar()->cookiesForUrl(MainUrl());
        for (auto &cookie : cookies) {
            if (QString(cookie.name()) == SessionIdCookie()) {
                _sessionId = cookie.value();
                emit LoginResult(0x00, "Logged in!");
                break;
            }
        }

        QNetworkRequest request = QNetworkRequest(AccountUrl());
        QNetworkReply *r = _manager->get(request);

        connect(r, &QNetworkReply::finished, this, &PSession::OnAccountPageResult);
    }
    reply->deleteLater();
}

void PSession::OnAccountPageResult() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
    if (reply->error()) {
        qDebug() << "Network error in " << __FUNCTION__ << ": " << reply->errorString();
    }
    else {
        // Regexp for getting last visited, guild, etc
        // <strong>(?<attr>.+?):<\/strong><br\/>\s+?(<a href=\"(?<url>.+?)\">(?<content1>.+?)<\/a>|\s+(?<content2>[A-Za-z0-9 ]+?)\s+<\/p>)


        const QByteArray data = reply->readAll();
        const QString avatar = GetAccountAvatar(data);
        const QString name = GetAccountName(data);
        const auto badges = GetAccountBadges(data);
        const int messages = GetAccountMessagesUnread(data);

        // Store the account name
        _accountName = name;

        QJsonObject object;
        object.insert("name", name);
        object.insert("avatar_url", avatar);
        object.insert("messages", messages);
        object.insert("badges", QJsonArray::fromStringList(badges.keys()));
        QJsonDocument temp(object);
        emit ProfileData(temp.toJson());

        // Request Avatar
        {
            const QString url = BaseUrl().toString() + avatar;
            const QString avatar = _cache->GenerateFileName(url);
            _avatars.append(avatar);
            _cache->GetImage(url);
        }
        // Request Badges
        for (const QString &key : badges.uniqueKeys()){
            const QString url = badges.value(key);
            if (url.isEmpty()) continue;
            GetAccountBadge(key, url);
        }
    }
    reply->deleteLater();
}

void PSession::OnLeaguesResult() {
    CHECK_REPLY;

    QByteArray response = reply->readAll();

    QJsonDocument doc = QJsonDocument::fromJson(response);
    QStringList list = doc.object().value("leagues").toVariant().toStringList();
    emit LeaguesList(list);

    reply->deleteLater();
}

void PSession::OnAccountStashTabsResult() {
    CHECK_REPLY;

    QByteArray response = reply->readAll();
    QNetworkRequest request = reply->request();
    QVariant data = GetAttribute(&request, UserData);
    QString league = GetAttribute(&request, League).toString();

    emit AccountStashTabs(league, response, data);
    QJsonDocument doc = QJsonDocument::fromJson(response);
    emit AccountStashTabsJson(league, doc, data);

    reply->deleteLater();
}

void PSession::OnAccountCharactersResult() {
    CHECK_REPLY;

    QByteArray response = reply->readAll();
    QNetworkRequest request = reply->request();
    QVariant data = GetAttribute(&request, UserData);

    emit AccountCharacters(response, data);
    QJsonDocument doc = QJsonDocument::fromJson(response);
    emit AccountCharactersJson(doc, data);

    reply->deleteLater();
}

void PSession::OnAccountCharacterItemsResult() {
    CHECK_REPLY;

    QByteArray response = reply->readAll();
    QNetworkRequest request = reply->request();
    QVariant data = GetAttribute(&request, UserData);
    QString character = GetAttribute(&request, Character).toString();

    emit AccountCharacterItems(character, response, data);
    QJsonDocument doc = QJsonDocument::fromJson(response);
    emit AccountCharacterItemsJson(character, doc, data);

    reply->deleteLater();
}

const QString PSession::GetCSRFToken(const QByteArray &data) {
    QRegularExpression expr("<input.+?name=\\\"hash\\\" value=\\\"(?<hash>.+?)\\\".+?id=\\\"hash\\\">");
    QRegularExpressionMatch match = expr.match(data);
    if (match.isValid() && match.hasMatch()) {
        return match.captured("hash");
    }
    return QString();
}

const QString PSession::GetAccountAvatar(const QByteArray &data) {
    QRegularExpression expr("<img\\s+src=\\\"(?<image>.+?)\\\"\\s+alt=\\\"Avatar\\\"");
    QRegularExpressionMatch match = expr.match(data);
    if (match.isValid() && match.hasMatch()) {
        return match.captured("image");
    }
    return QString();
}

const QString PSession::GetAccountName(const QByteArray &data) {
    QRegularExpression expr("<h1\\s+class=\\\"name\\\">(?<name>.+?)<\\/h1>");
    QRegularExpressionMatch match = expr.match(data);
    if (match.isValid() && match.hasMatch()) {
        return match.captured("name");
    }
    return QString();
}

int PSession::GetAccountMessagesUnread(const QByteArray &data) {
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

const QMap<QString, QString> PSession::GetAccountBadges(const QByteArray &data) {
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

