#include "setupdialog.h"
#include "ui_setupdialog.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QStandardPaths>
#include <core.h>
#include <session/sessionrequest.h>

#include <ui/dialogs/loginsessiondialog.h>

#undef STEAM_SUPPORT

SetupDialog::SetupDialog(QWidget *parent, CoreService* core)
    : QDialog(parent)
    , ui(new Ui::SetupDialog)
    , _core(core)
    , _email()
    , _sessionId()
    , _accountName("Unknown")
    , _poePath()
    , _poeConfigPath()
    , _analytics(false)
{
    ui->setupUi(this);
    //setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
#ifndef STEAM_SUPPORT
    ui->steamLoginButton->hide();
#endif
    setPage(SetupDialog::LoginMethodPage);

    _loginMethods.insert(SetupDialog::LoginEmail, new LoginSessionDialog(this, core));
}

void SetupDialog::closeEvent(QCloseEvent* event) {
    QMessageBox dialog(QMessageBox::Question, "Adamant Setup", "Are you sure you want to quit the setup?", QMessageBox::Yes | QMessageBox::No, this);
    if (dialog.exec() == QMessageBox::Yes) {
        setResult(0x1);
        event->accept();
        return;
    }
    event->ignore();
}

SetupDialog::~SetupDialog()
{
    delete ui;
}

void SetupDialog::setPage(SetupDialog::Page p) {
    ui->stackWidget->setCurrentIndex(p);
    // Disallow "backing" into login
    ui->backButton->setHidden(p <= SetupDialog::AccountPage);
}

void SetupDialog::loginSuccess(const QString &sessionId) {
    auto dialog = _loginMethods.value(_method);
    if (dialog) {
        dialog->accept();
    }
    setPage(SetupDialog::AccountPage);
    _sessionId = sessionId;
}

void SetupDialog::loginFailed(const QString &message) {
    auto dialog = _loginMethods.value(_method);
    if (dialog) {
        dialog->showError(message);
    }
}

void SetupDialog::updateAccountAvatar(QImage image) {
    QPixmap pixmap = QPixmap::fromImage(image);
    ui->accountAvatar->setPixmap(pixmap);
}

void SetupDialog::updateAccountName(const QString &name) {
    _accountName = name;
    ui->welcomeLabel->setText(QString("Welcome %1").arg(_accountName));
}

void SetupDialog::on_poeLoginButton_clicked() {
    attemptLogin(SetupDialog::LoginEmail);
}

void SetupDialog::on_sessionIdLoginButton_clicked() {
    attemptLogin(SetupDialog::LoginSessionId);
}

void SetupDialog::attemptLogin(LoginMethod method) {
    _method = method;
    auto dialog = _loginMethods.value(_method);
    if (dialog) {
        dialog->exec();
    }
}

void SetupDialog::on_continueNameButton_clicked()
{
    setPage(SetupDialog::PathsPage);
}

void SetupDialog::on_continuePathsButton_clicked()
{
    setPage(SetupDialog::AnalyticsPage);
}

void SetupDialog::on_continueAnalyticsButton_clicked()
{
    setPage(SetupDialog::ImportPage);
}

void SetupDialog::on_finishButton_clicked() {
    // Success
    done(0x0);
}

void SetupDialog::on_backButton_clicked() {
    setPage((SetupDialog::Page) (ui->stackWidget->currentIndex() - 1));
}

void SetupDialog::on_skipButton_clicked() {
    setPage(SetupDialog::FinishPage);
}

void SetupDialog::on_importButton_clicked() {
    setPage(SetupDialog::FinishPage);
}

void SetupDialog::on_poeConfigBrowseButton_clicked() {
    QString directory = QFileDialog::getOpenFileName(this, "Adamant - Path of Exile Config Directory", QString(),
                                                     "Config (production_Config.ini)");
    if (!directory.isEmpty()) {
        _poeConfigPath = QFileInfo(directory).absolutePath();
        ui->poeConfigEdit->setText(_poeConfigPath);
    }
}

void SetupDialog::on_poeBrowseButton_clicked() {
    QString directory = QFileDialog::getOpenFileName(this, "Adamant - Path of Exile Directory", QString(),
                                                     "Game (PathOfExile.exe PathOfExileSteam.exe)");
    if (!directory.isEmpty()) {
        _poePath = QFileInfo(directory).absolutePath();
        ui->poeEdit->setText(_poePath);
    }
}

void SetupDialog::on_analyticsCheckbox_toggled(bool checked) {
    _analytics = checked;
}
