#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>
#include <QVariantMap>

namespace Ui {
class SetupDialog;
}

class CoreService;
class ILoginDialog;

class SetupDialog : public QDialog
{
    Q_OBJECT
public:
    enum Page {
        LoginMethodPage,
        AccountPage,
        PathsPage,
        AnalyticsPage,
        ImportPage,
        FinishPage
    };

    enum LoginMethod {
        LoginNone,
        LoginSessionId,
        LoginOAuth
    };

public:
    explicit SetupDialog(QWidget *parent, CoreService* core);
    ~SetupDialog();

    void setPage(Page p);

    QVariantMap getData() const {
        QVariantMap map;
        map.insert("id", _sessionId);
        map.insert("access_token", _accessToken);
        map.insert("method", (int)_method);
        map.insert("account", _accountName);
        map.insert("poe", _poePath);
        map.insert("poe_config", _poeConfigPath);
        map.insert("analytics", _analytics);
        return map;
    }

    void attemptLogin(LoginMethod method);
public slots:
    void loginSuccess(const QString &sessionId);
    void loginFailed(const QString &message);
    void updateAccountAvatar(QImage image);
    void updateAccountName(const QString &name);
protected:
    void closeEvent(QCloseEvent *event);
private slots:
    void on_sessionIdLoginButton_clicked();
    void on_continueNameButton_clicked();
    void on_continuePathsButton_clicked();
    void on_continueAnalyticsButton_clicked();
    void on_finishButton_clicked();
    void on_skipButton_clicked();
    void on_importButton_clicked();
    void on_backButton_clicked();

    void on_poeConfigBrowseButton_clicked();

    void on_poeBrowseButton_clicked();

    void on_analyticsCheckbox_toggled(bool checked);

    void on_oauthLoginButton_clicked();

signals:
    void loginByIdRequested(const QString &sessionId);
private:
    Ui::SetupDialog *ui;
    CoreService* _core;

    LoginMethod _method;
    QString _sessionId;
    QString _accessToken;
    QString _accountName;
    QString _poePath;
    QString _poeConfigPath;
    bool _analytics;

    QMap<LoginMethod, ILoginDialog*> _loginMethods;
};
Q_DECLARE_METATYPE(SetupDialog*)

#endif // SETUPDIALOG_H
