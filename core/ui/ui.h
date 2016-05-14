#ifndef UI_H
#define UI_H

#include <core_global.h>
#include <QObject>
#include <QApplication>
#include <QDateTime>

#include <ui/mainwindow.h>
#include <ui/setupdialog.h>

#include <ui/pages/pluginpage.h>

class CORE_EXTERN AdamantUI : public QObject
{
    Q_OBJECT
public:
    AdamantUI(CoreService* parent);
    ~AdamantUI();
    Q_PROPERTY(SetupDialog* SetupDialog MEMBER _setupDialog)
    Q_PROPERTY(MainWindow* MainWindow MEMBER _window)
    Q_PROPERTY(ApplicationTheme Theme MEMBER _theme NOTIFY applicationThemeChanged)

    enum class ApplicationTheme {
        Light = 1,
        Dark = 2
    };
    Q_ENUM(ApplicationTheme)

    ApplicationTheme getTheme() {
        return _theme;
    }

    MainWindow* window() {
        return _window;
    }

    SetupDialog* getSetupDialog() {
        return _setupDialog;
    }

    void start();
    int showSetup();
    Q_INVOKABLE void setTheme(ApplicationTheme theme = ApplicationTheme::Dark);

    void registerPages() {
        // TODO(rory): implement these as plugins
//        Window()->RegisterPage(QIcon(":/icons/dark/coin-dollar.png"), "Currency", "View currency and statistics.",
//                                    new QWidget(), true);
//        Window()->RegisterPage(QIcon(":/icons/dark/target.png"), "Recipes", "View recipes.",
//                                    new QWidget(), true);
//        Window()->RegisterPage(QIcon(":/icons/dark/filter.png"), "Loot Filters", "Manage lootfilters.",
//                                    new QWidget(), true);

        window()->registerPage(QIcon(":/icons/dark/make-group.png"), "Plugins", "Manage plugins and updates.",
                                    new PluginPage(_core, window()), true);
    }

signals:
    void requestProfileData(QString sessionId);
    void applicationThemeChanged(ApplicationTheme old, ApplicationTheme now);
private:
    MainWindow* _window;
    SetupDialog* _setupDialog;
    CoreService* _core;
    ApplicationTheme _theme;
    QPalette _lightPalette;
    QPalette _darkPalette;
};
Q_DECLARE_METATYPE(AdamantUI*)
Q_DECLARE_METATYPE(AdamantUI::ApplicationTheme)

#endif // UI_H

