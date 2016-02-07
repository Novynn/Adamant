#ifndef SESSION_H
#define SESSION_H

#include <core_global.h>
#include <QUrl>

class CoreService;

class CORE_EXTERN Session
{
public:
    class Request;

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

    static void SetCoreService(CoreService* core);

    static void LogError(const QString &error);

    static Request* Global();

    static Request* NewRequest(QObject* parent = 0);

private:
    static Request* _globalRequest;
    static CoreService* _core;

    explicit Session() {}
};

#endif // SESSION_H
