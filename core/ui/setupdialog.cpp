#include "setupdialog.h"
#include "ui_setupdialog.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QStandardPaths>

#undef STEAM_SUPPORT

SetupDialog::SetupDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SetupDialog)
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
    ui->backButton->setHidden(p == 0);

    if (p == SetupDialog::LoginPage) {
        UpdateLoginInput("");
    }
}

void SetupDialog::LoginSuccess(const QString &sessionId) {
    UpdateLoginInput("Login Successful", true);
    ui->sessionIdEdit->clear();
    setPage(SetupDialog::AccountPage);
    _sessionId = sessionId;
}

void SetupDialog::LoginFailed(const QString &message) {
    UpdateLoginInput("Login Failed: " + message, true);
}

void SetupDialog::UpdateAccountAvatar(QImage image) {
    QPixmap pixmap = QPixmap::fromImage(image);
    ui->accountAvatar->setPixmap(pixmap);
}

void SetupDialog::UpdateAccountName(const QString &name) {
    _accountName = name;
    ui->welcomeLabel->setText(QString("Welcome %1").arg(_accountName));
}

void SetupDialog::on_poeLoginButton_clicked() {
    ui->loginInputWidget->show();
    ui->sessionInputWidget->hide();
    _method = LoginEmail;
    setPage(SetupDialog::LoginPage);
}

void SetupDialog::on_sessionIdLoginButton_clicked() {
    ui->loginInputWidget->hide();
    ui->sessionInputWidget->show();
    _method = LoginSessionId;
    setPage(SetupDialog::LoginPage);
}

void SetupDialog::on_loginButton_clicked() {
    switch (_method) {
        case LoginEmail: {
            QString email = ui->emailEdit->text();
            QString password = ui->passwordEdit->text();

            if (!email.isEmpty() && !password.isEmpty()) {
                _email = email;
                UpdateLoginInput("Logging In...", false);

                emit LoginRequested(email, password);
                ui->passwordEdit->clear();
            }
            else {
                UpdateLoginInput("Please enter your email and your password.", true);
            }
        } break;
        case LoginSessionId: {
            QString id = ui->sessionIdEdit->text();

            if (!id.isEmpty() && id.length() == 32) {
                _email.clear();
                UpdateLoginInput("Logging In...", false);

                emit LoginByIdRequested(id);
            }
            else {
                UpdateLoginInput("Your session ID is invalid.", true);
            }
        } break;
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

void SetupDialog::UpdateLoginInput(const QString &message, bool enable) {
    ui->loginStatusLabel->setText(message);
    ui->emailEdit->setEnabled(enable);
    ui->passwordEdit->setEnabled(enable);
    ui->loginButton->setEnabled(enable);
    ui->backButton->setEnabled(enable);
}

void SetupDialog::on_skipButton_clicked() {
    setPage(SetupDialog::FinishPage);
}

void SetupDialog::on_importButton_clicked() {
    setPage(SetupDialog::FinishPage);
}

void SetupDialog::on_changeNameButton_clicked(){
   bool ok;
   QString text = QInputDialog::getText(this, "Adamant - Account Name Input",
                                        "Account Name:", QLineEdit::Normal,
                                        _accountName, &ok);
   if (ok && !text.isEmpty()) {
       UpdateAccountName(text);
   }
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

void SetupDialog::on_passwordEdit_returnPressed() {
    ui->loginButton->setFocus();
    ui->loginButton->click();
}

void SetupDialog::on_emailEdit_returnPressed() {
    ui->passwordEdit->setFocus();
    ui->passwordEdit->selectAll();
}

void SetupDialog::on_sessionIdEdit_editingFinished() {
    ui->loginButton->setFocus();
    ui->loginButton->click();
}
