#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <core_global.h>
#include <QMainWindow>
#include <QMap>
#include <QProgressBar>
#include <QMovie>
#include <QUuid>

namespace Ui {
class MainWindow;
}

class CoreService;
class QLabel;
class CommandButton;
class CustomPage;

class CORE_EXTERN MainWindow : public QMainWindow
{
    Q_OBJECT

    enum Mode {
        InvalidMode,
        LoadingMode,
        HomeMode,
        ElsewhereMode
    };

public:
    explicit MainWindow(CoreService *core, QWidget *parent = 0);
    ~MainWindow();

    void setPageIndex(int index);

    Q_INVOKABLE QUuid registerPage(const QIcon &icon, const QString &title, const QString &description,
                                 QWidget* widget, QObject* owner = nullptr);
    void setCurrentPageButton(int index);
    void setMenuExpanded(bool expanded);
    Q_INVOKABLE bool removePage(QUuid id);
    void setLoginProgressMessage(const QString &message);
public slots:
    void onProfileBadgeImage(const QString &badge, QImage image);
    void updateAccountMessagesCount(int messages);
    void appendScriptOutput(const QString &output, const QString &type = "LOG");
    void setPageByUuid(QUuid id);
    void regeneratePageIndex();
private slots:
    void on_lineEdit_returnPressed();
    void on_messagesButton_clicked();
    void on_reloadScriptsButton_clicked();
    void on_homeButton_clicked();
    void on_toggleButton_toggled(bool checked);
protected:
    bool eventFilter(QObject *obj, QEvent *event);
signals:
    void loaded();
private:
    Ui::MainWindow *ui;
    CoreService* _core;

    void setMode(Mode mode);

    Mode _mode;

    QMap<QString, QLabel*> _badgeMap;

    QMap<QUuid, CustomPage*> _customPages;
    QMap<QUuid, int> _customPagesIndex;
    int _shortcutIndex = 0;

    QLabel* _statusBarLabel;
    QProgressBar* _statusBarProgress;

    QMovie* _loadingImage;

    int _currentIndex = -1;
    QStringList _scriptHistory;
};
Q_DECLARE_METATYPE(MainWindow*)

#endif // MAINWINDOW_H
