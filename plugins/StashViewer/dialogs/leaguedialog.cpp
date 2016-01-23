#include "leaguedialog.h"
#include "ui_leaguedialog.h"

#include <QDebug>

LeagueDialog::LeagueDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LeagueDialog) {
    ui->setupUi(this);
    ui->checkBox->setChecked(false);

}

LeagueDialog::~LeagueDialog()
{
    delete ui;
}

void LeagueDialog::SetLeagues(QStringList leagues) {
    _leagues = leagues;

    ui->leaguesListWidget->clear();
    ui->leaguesListWidget->addItems(leagues);
}

void LeagueDialog::SetLeagueTabs(QString league, QStringList tabs) {
    QList<QListWidgetItem*> selection = ui->leaguesListWidget->selectedItems();
    if (selection.isEmpty()) return;
    if (league == selection.first()->text()) {
        ui->tabsListWidget->clear();
        ui->tabsListWidget->addItems(tabs);
        ui->leaguesListWidget->setEnabled(true);

        emit ui->checkBox->toggled(ui->checkBox->isChecked());
    }
}

void LeagueDialog::on_checkBox_toggled(bool checked) {
    static const Qt::ItemFlags DefaultFlags = Qt::ItemIsSelectable |
                                              Qt::ItemIsUserCheckable |
                                              Qt::ItemIsEnabled |
                                              Qt::ItemIsDragEnabled;

    for (int i = 0; i < ui->tabsListWidget->count(); i++) {
        QListWidgetItem* item = ui->tabsListWidget->item(i);
        if (!checked) {
            // Default state
            item->setFlags(DefaultFlags);
            item->setCheckState(Qt::Checked);
        }
        else {
            // Clear checkbox
            item->setData(Qt::CheckStateRole, QVariant());
        }
    }

    ui->advancedWidget->setVisible(checked);
    if (checked) {
        emit ui->advancedEdit->textChanged(ui->advancedEdit->text());
    }
    else {
        _currentFilter = "";
    }
}

void LeagueDialog::on_advancedEdit_textChanged(const QString &text) {
    static const Qt::ItemFlags DefaultFlags = Qt::ItemIsSelectable |
                                              Qt::ItemIsUserCheckable |
                                              Qt::ItemIsEnabled |
                                              Qt::ItemIsDragEnabled;

    QRegularExpression expr(text, QRegularExpression::CaseInsensitiveOption);
    for (int i = 0; i < ui->tabsListWidget->count(); i++) {
        QListWidgetItem* item = ui->tabsListWidget->item(i);
        if (!text.isEmpty() &&
            expr.isValid() &&
            item->text().contains(expr)) {
            item->setFlags(Qt::NoItemFlags);
        }
        else {
            item->setFlags(DefaultFlags);
        }
    }
    _currentFilter = text;
}

void LeagueDialog::on_leaguesListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous) {
    Q_UNUSED(previous)
    if (!current || current->text().isEmpty()) return;
    _currentLeague = current->text();
    ui->tabsListWidget->clear();
    ui->leaguesListWidget->setEnabled(false);
    emit RequestStashTabList(_currentLeague);
}
