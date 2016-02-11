#include "leaguedialog.h"
#include "ui_leaguedialog.h"

#include <QDebug>

LeagueDialog::LeagueDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LeagueDialog) {
    ui->setupUi(this);
    ui->splitter->setStretchFactor(0, 2);
    ui->splitter->setStretchFactor(1, 1);
}

LeagueDialog::~LeagueDialog()
{
    delete ui;
}

void LeagueDialog::SetLeagues(QStringList leagues) {
    ui->tableWidget->clearContents();
    while (ui->tableWidget->rowCount()) {
        ui->tableWidget->removeRow(0);
    }
    for (QString league : leagues) {
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);

        auto header = new QTableWidgetItem(league);
        header->setFlags(header->flags() & ~Qt::ItemIsEditable);
        ui->tableWidget->setItem(row, 0, header);
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(""));
    }
}

void LeagueDialog::SetLeagueTabs(QString league, QStringList tabs) {
    _leagueTabs.remove(league);
    _leagueTabs.insert(league, tabs);

    if (league == _currentLeague) {
        ui->tabsListWidget->clear();
        ui->tabsListWidget->addItems(tabs);
        ui->tableWidget->setEnabled(true);
        applyFilter();
    }
}

void LeagueDialog::applyFilter() {
    QString regex = _leagueFilters.value(_currentLeague);
    QRegularExpression expr(regex);
    if (regex.isEmpty() || !expr.isValid()) {
        return;
    }

    for (int i = 0; i < ui->tabsListWidget->count(); i++) {
        QListWidgetItem* item = ui->tabsListWidget->item(i);
        auto match = expr.match(item->text());
        if (match.hasMatch()) {
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        }
        else {
            item->setFlags(item->flags() & Qt::ItemIsEnabled);
        }
    }
}

void LeagueDialog::Clear() {
    SetLeagues({});
    _leagueTabs.clear();
    ui->tableWidget->setEnabled(true);
}

void LeagueDialog::on_tableWidget_cellChanged(int row, int column) {
    if (column != 1) return;

    QString league = ui->tableWidget->item(row, 0)->text();
    auto regexItem = ui->tableWidget->item(row, column);
    QString regex = regexItem->text();
    _leagueFilters.remove(league);
    _leagueFilters.insert(league, regex);

    applyFilter();
}


void LeagueDialog::on_tableWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn) {
    Q_UNUSED(currentColumn)
    Q_UNUSED(previousRow)
    Q_UNUSED(previousColumn)

    if (currentRow < 0) {
        ui->selectButton->setEnabled(false);
        ui->tabsListWidget->clear();
        _currentLeague = "";
        return;
    }
    ui->selectButton->setEnabled(true);

    QString league = ui->tableWidget->item(currentRow, 0)->text();
    _currentLeague = league;
    if (_leagueTabs.value(league).isEmpty()) {
        ui->tableWidget->setEnabled(false);
        ui->tabsListWidget->clear();
        RequestStashTabList(league);
    }
    else {
        SetLeagueTabs(league, _leagueTabs.value(league));
    }
}

void LeagueDialog::on_selectButton_clicked() {
    accept();
}

void LeagueDialog::on_tableWidget_cellDoubleClicked(int row, int column) {
    Q_UNUSED(column)

    if (!ui->tableWidget->isEnabled()) return;

    QString league = ui->tableWidget->item(row, 0)->text();
    _currentLeague = league;
    accept();
}
