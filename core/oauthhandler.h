#ifndef OAUTHHANDLER_H
#define OAUTHHANDLER_H

#include <QObject>
#include <QSettings>
#include <QTcpServer>

class OAuthHandler : public QObject {
    Q_OBJECT
public:
    OAuthHandler();

    bool listen();
    void signIn(bool force = false);

    const QString getAccessToken() const {
        return _token;
    }

    bool hasAccessToken() const {
        return !_token.isEmpty();
    }

    void save(const QString &accessToken);
protected:
    QString _token;
    QString _code;
private:
    QSharedPointer<QTcpServer> _server;

    QString state;
    QSettings _sensitiveSettings;
signals:
    void oauthCodeAcquired(const QString &);
    void oauthAccessTokenAcquired(const QString &);
};

#endif // OAUTHHANDLER_H
