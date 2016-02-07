#ifndef SETUPDIALOG_H
#define SETUPDIALOG_H

#include <QDialog>
#include <QVariantMap>

namespace Ui {
class SetupDialog;
}

class SetupDialog : public QDialog
{
    Q_OBJECT

    enum Page {
        LoginMethodPage,
        LoginPage,
        AccountPage,
        PathsPage,
        AnalyticsPage,
        ImportPage,
        FinishPage
    };

    enum LoginMethod {
        LoginEmail,
        LoginSessionId,
        LoginSteam
    };

public:
    explicit SetupDialog(QWidget *parent = 0);
    ~SetupDialog();

    void setPage(Page p);

    QVariantMap getData() const {
        QVariantMap map;
        map.insert("id", _sessionId);
        map.insert("email", _email);
        map.insert("method", (int)_method);
        map.insert("account", _accountName);
        map.insert("poe", _poePath);
        map.insert("poe_config", _poeConfigPath);
        map.insert("analytics", _analytics);
        return map;
    }

public slots:
    void loginSuccess(const QString &sessionId);
    void loginFailed(const QString &message);
    void updateAccountAvatar(QImage image);
    void updateAccountName(const QString &name);
protected:
    void closeEvent(QCloseEvent *event);
private slots:
    void on_poeLoginButton_clicked();
    void on_sessionIdLoginButton_clicked();
    void on_loginButton_clicked();
    void on_continueNameButton_clicked();
    void on_continuePathsButton_clicked();
    void on_continueAnalyticsButton_clicked();
    void on_finishButton_clicked();
    void on_skipButton_clicked();
    void on_importButton_clicked();
    void on_backButton_clicked();
    void on_changeNameButton_clicked();

    void on_poeConfigBrowseButton_clicked();

    void on_poeBrowseButton_clicked();

    void on_analyticsCheckbox_toggled(bool checked);

    void on_passwordEdit_returnPressed();

    void on_emailEdit_returnPressed();

    void on_sessionIdEdit_editingFinished();

signals:
    void loginRequested(const QString &username, const QString &password);
    void loginByIdRequested(const QString &sessionId);
private:
    Ui::SetupDialog *ui;

    LoginMethod _method;
    QString _email;
    QString _sessionId;
    QString _accountName;
    QString _poePath;
    QString _poeConfigPath;
    bool _analytics;

    void updateLoginInput(const QString &message, bool enable=true);
};
Q_DECLARE_METATYPE(SetupDialog*)

#endif // SETUPDIALOG_H
