#ifndef LEAGUEDIALOG_H
#define LEAGUEDIALOG_H

#include <QDialog>
#include <QListWidget>

namespace Ui {
class LeagueDialog;
}

class LeagueDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LeagueDialog(QWidget *parent = 0);
    ~LeagueDialog();

    void SetLeagues(QStringList leagues);
    void SetLeagueTabs(QString league, QStringList tabs);

    QString GetChosenLeague() {
        return _currentLeague;
    }

    QString GetFilter() {
        return _leagueFilters.value(_currentLeague);
    }

    void applyFilter();

    void Clear();

private slots:

    void on_tableWidget_cellChanged(int row, int column);
    void on_tableWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    void on_selectButton_clicked();

    void on_tableWidget_cellDoubleClicked(int row, int column);

signals:
    void RequestStashTabList(QString league);
private:
    Ui::LeagueDialog *ui;

    QMap<QString, QStringList> _leagueTabs;
    QMap<QString, QString> _leagueFilters;

    QString _currentLeague;
};

#endif // LEAGUEDIALOG_H
