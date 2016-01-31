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
        return _currentFilter;
    }

    void Clear();

private slots:
    void on_checkBox_toggled(bool checked);

    void on_advancedEdit_textChanged(const QString &text);
    void on_leaguesListWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

signals:
    void RequestStashTabList(QString league);
private:
    Ui::LeagueDialog *ui;

    QStringList _leagues;
    QString _currentLeague;
    QString _currentFilter;
};

#endif // LEAGUEDIALOG_H
