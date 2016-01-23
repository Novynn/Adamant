#ifndef UI_H
#define UI_H

#include <core_global.h>
#include <QObject>
#include <QApplication>
#include <QDateTime>

#include <ui/mainwindow.h>
#include <ui/setupdialog.h>

#include <ui/pages/pluginpage.h>

class CORE_EXTERN UI : public QObject
{
    Q_OBJECT
public:
    UI(CoreService* parent);
    Q_PROPERTY(SetupDialog* SetupDialog MEMBER _setupDialog)
    Q_PROPERTY(MainWindow* MainWindow MEMBER _window)

    MainWindow* Window() {
        return _window;
    }

    SetupDialog* GetSetupDialog() {
        return _setupDialog;
    }

    void OnLoad();
    int ShowSetup();
    void SetPalette();

    void RegisterPages() {
        // TODO(rory): implement these as plugins
//        Window()->RegisterPage(QIcon(":/icons/dark/cart.png"), "Shops", "Manage shop threads.",
//                                    new QWidget(), true);
//        Window()->RegisterPage(QIcon(":/icons/dark/coin-dollar.png"), "Currency", "View currency and statistics.",
//                                    new QWidget(), true);
//        Window()->RegisterPage(QIcon(":/icons/dark/target.png"), "Recipes", "View recipes.",
//                                    new QWidget(), true);
//        Window()->RegisterPage(QIcon(":/icons/dark/filter.png"), "Loot Filters", "Manage lootfilters.",
//                                    new QWidget(), true);

        Window()->RegisterPage(QIcon(":/icons/dark/make-group.png"), "Plugins", "Manage plugins and updates.",
                                    new PluginPage(_core, Window()), true);
    }

signals:
    void RequestProfileData(QString sessionId);
private:
    MainWindow* _window;
    SetupDialog* _setupDialog;
    CoreService* _core;
};
Q_DECLARE_METATYPE(UI*)

#endif // UI_H

