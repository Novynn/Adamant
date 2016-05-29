#include "newshopdialog.h"
#include "ui_newshopdialog.h"

NewShopDialog::NewShopDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewShopDialog) {
    ui->setupUi(this);
    ui->badNameLabel->hide();
}

NewShopDialog::~NewShopDialog() {
    delete ui;
}

QString NewShopDialog::name() {
    return ui->nameEdit->text();
}

QString NewShopDialog::league() {
    return ui->shopLeagueBox->currentText();
}

void NewShopDialog::accept() {
    if (_badNames.contains(name(), Qt::CaseInsensitive)) {
        ui->badNameLabel->show();
        ui->nameEdit->setFocus();
        ui->nameEdit->selectAll();
        return;
    }
    QDialog::accept();
}

void NewShopDialog::setLeagues(const QStringList &leagues) {
    ui->shopLeagueBox->clear();
    ui->shopLeagueBox->addItems(leagues);
}

void NewShopDialog::setBadNames(const QStringList& names) {
    _badNames = names;
}

void NewShopDialog::reset() {
    ui->nameEdit->clear();
    _badNames.clear();
}

void NewShopDialog::on_nameEdit_textChanged(const QString &text) {
    ui->badNameLabel->setVisible(_badNames.contains(text, Qt::CaseInsensitive));
}
