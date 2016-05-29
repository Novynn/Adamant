#ifndef NEWSHOPDIALOG_H
#define NEWSHOPDIALOG_H

#include <QDialog>

namespace Ui {
class NewShopDialog;
}

class NewShopDialog : public QDialog {
    Q_OBJECT

public:
    explicit NewShopDialog(QWidget *parent = 0);
    ~NewShopDialog();

    QString name();
    QString league();

public slots:
    void accept();
    void setLeagues(const QStringList& leagues);
    void setBadNames(const QStringList& names);
    void reset();
private slots:
    void on_nameEdit_textChanged(const QString &text);

private:
    Ui::NewShopDialog *ui;
    QStringList _badNames;
};

#endif // NEWSHOPDIALOG_H
