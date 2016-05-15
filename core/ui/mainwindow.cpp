#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "core.h"
#include "ui/ui.h"
#include "commandbutton.h"
#include <QDesktopServices>
#include <QShortcut>
#include "pluginmanager.h"
#include "scripting/scriptsandbox.h"
#include <items/itemmanager.h>

MainWindow::MainWindow(CoreService *core, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , _core(core)
    , _mode(InvalidMode)
    , _loadingImage(new QMovie(":/images/loading_dark.gif"))
{
    ui->setupUi(this);
    ui->loadingLabel->setMovie(_loadingImage);
    ui->consoleButton->click();

    setMode(LoadingMode);

    _buttonForPage.insert(0, ui->homeButton);

    connect(_core->script(), &ScriptSandbox::scriptOutput, [this] (const QString &message) {
        appendScriptOutput(message, "===");
    });

    ui->lineEdit->installEventFilter(this);

    _statusBarLabel = new QLabel(statusBar());
    _statusBarLabel->setMargin(8);
    _statusBarProgress = new QProgressBar(statusBar());
    _statusBarProgress->setAlignment(Qt::AlignLeft);
    _statusBarProgress->setMaximumSize(256, 19);
    _statusBarProgress->setMaximum(100);
    _statusBarProgress->setFormat("%v/%m");
    statusBar()->addWidget(_statusBarLabel);
    statusBar()->addPermanentWidget(_statusBarProgress);
    _statusBarProgress->hide();

//    connect(_core->getItemManager(), &ItemManager::onStashTabUpdateBegin, [this](QString league) {
//        const QString message = QString("Loading %1 stash tabs...").arg(league);
//        _statusBarLabel->setText(message);
//        qInfo() << qPrintable(message);
//    });

//    connect(_core->getItemManager(), &ItemManager::onStashTabUpdateProgress, [this](QString league, QString id, bool throttled) {
//        _statusBarProgress->show();
//        _statusBarProgress->setValue(r);
//        _statusBarProgress->setMaximum(m);
//        QString message;
//        if (t)
//            message = QString("Loading %1 stash tabs... (throttled)").arg(league);
//        else
//            message = QString("Loading %1 stash tabs... (%2/%3)").arg(league).arg(r).arg(m);

//        _statusBarLabel->setText(message);
//    });

//    connect(_core->getItemManager(), &ItemManager::onStashTabUpdateAvailable, [this] (QString league) {
//        _statusBarProgress->setValue(0);
//        _statusBarProgress->setMaximum(100);
//        _statusBarProgress->hide();
//        const QString message = QString("%1 stash tabs loaded!").arg(league);
//        _statusBarLabel->setText(message);
//        qInfo() << qPrintable(message);
//    });

    {
        QShortcut* shortcut = new QShortcut(QKeySequence("Ctrl+`"), this);
        connect(shortcut, &QShortcut::activated, [this] () {
            if (_mode == HomeMode || _mode == ElsewhereMode)
                ui->toggleButton->toggle();
        });
    }

    {
        QShortcut* shortcut = new QShortcut(QKeySequence("Ctrl+="), this);
        connect(shortcut, &QShortcut::activated, [this] () {
            if (_mode == HomeMode || _mode == ElsewhereMode)
                setMode(LoadingMode);
            else
                setMode(HomeMode);
        });
    }

    {
        QShortcut* shortcut = new QShortcut(QKeySequence("Ctrl+R"), this);
        connect(shortcut, &QShortcut::activated, [this] () {
            _core->getPluginManager()->reloadScripts();
            appendScriptOutput("Scripts reloaded!");
        });
    }

    {
        QShortcut* shortcut = new QShortcut(QKeySequence("Ctrl+" + QString::number(++_shortcutIndex)), this);
        connect(shortcut, &QShortcut::activated, [this] () {
            if (_mode == HomeMode || _mode == ElsewhereMode)
                setPageIndex(0);
        });
    }

    setMenuExpanded(false);

    emit loaded();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::appendScriptOutput(const QString &output, const QString &type) {
    const QString chopped = QString("%1").arg(type, 3, QChar('-'));
    QString pre = output.toHtmlEscaped();
    pre.replace(" ", "&nbsp;");
    pre.replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;");
    pre.replace("\n", "<br>");
    ui->plainTextEdit->appendHtml(QString("<font color='grey'>%1</font> %2").arg(chopped).arg(pre));
}

void MainWindow::on_lineEdit_returnPressed() {
    QString text = ui->lineEdit->text();
    ui->lineEdit->clear();

    appendScriptOutput(text, "-->");

    // Reset index
    _currentIndex = -1;
    if (_scriptHistory.isEmpty() || (_scriptHistory.first() != text)) {
        _scriptHistory.prepend(text);
        while (_scriptHistory.size() >= 100) _scriptHistory.takeLast();
    }
    _core->script()->addLine(text);
}

void MainWindow::onProfileBadgeImage(const QString &badge, QImage image) {
    QLabel* label;
    if (_badgeMap.contains(badge)) {
        label = _badgeMap.value(badge);
        label->setPixmap(QPixmap::fromImage(image));
    }
    else {
        label = new QLabel(ui->badgeWidget);
        label->setPixmap(QPixmap::fromImage(image));
        label->setScaledContents(true);
        label->setAlignment(Qt::AlignCenter);
        ui->badgeWidget->layout()->addWidget(label);
        _badgeMap.insert(badge, label);
    }
}

void MainWindow::updateAccountMessagesCount(int messages) {
    ui->messagesButton->setText(QString("Private Messages (%1 unread)").arg(messages));
    ui->messagesButton->setVisible(messages > 0);
}

void MainWindow::on_messagesButton_clicked() {
    QDesktopServices::openUrl(QUrl("https://www.pathofexile.com/private-messages/inbox"));
}

void MainWindow::on_reloadScriptsButton_clicked() {
    _core->getPluginManager()->reloadScripts();
    appendScriptOutput("Scripts reloaded!");
}

void MainWindow::on_homeButton_clicked() {
    setPageIndex(0);
}

void MainWindow::setCurrentPageButton(int index) {
    for (int page : _buttonForPage.keys()) {
        CommandButton* button = _buttonForPage.value(page);
        if (button) {
            bool isSelectedIndex = page == index;
            button->setChecked(isSelectedIndex);
        }
    }
}

void MainWindow::setPageIndex(int index) {
    if (index == 0 || index > ui->stackedWidget->count()) {
        setMode(HomeMode);
        ui->stackedWidget->setCurrentIndex(0);
        setCurrentPageButton(0);
        return;
    }
    setMode(ElsewhereMode);
    ui->stackedWidget->setCurrentIndex(index);
    setCurrentPageButton(index);
}

int MainWindow::registerPage(const QIcon &icon, const QString &title, const QString &description,
                                         QWidget *widget, bool lower) {
    int initialCount = ui->stackedWidget->count();
    int index = ui->stackedWidget->addWidget(widget);

    // If the amount of pages changed, then the widget was new.
    // Otherwise, the widget is already registered and we don't need to remake the button.
    if (initialCount != ui->stackedWidget->count()) {
        CommandButton* button = new CommandButton(widget);
        button->setText(title);
        button->setDescription(description);
        button->setToolTip(description);
        button->setIcon(icon);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        button->setMinimumSize(30, 36);
        button->setMaximumSize(16777215, 36);
        button->setCheckable(true);

        _buttonForPage.insert(index, button);
        if (lower) {
            ui->lowerNavWidget->layout()->addWidget(button);
        }
        else {
            ui->navWidget->layout()->addWidget(button);

            QShortcut* shortcut = new QShortcut(QKeySequence("Ctrl+" + QString::number(++_shortcutIndex)), this);
            connect(shortcut, &QShortcut::activated, [this, index] () {
                if (_mode == HomeMode || _mode == ElsewhereMode)
                    setPageIndex(index);
            });
        }

        button->setIconOnly(!ui->toggleButton->isChecked());

        connect(button, &CommandButton::clicked, [this, index] () {
            setPageIndex(index);
        });
    }

    return index;
}

void MainWindow::setMenuExpanded(bool expanded) {
    for (CommandButton* button : ui->sidebarWidget->findChildren<CommandButton*>()) {
        button->setIconOnly(!expanded);
    }
    if (expanded) {
        ui->sidebarWidget->setMinimumSize(240, 0);
        ui->sidebarWidget->setMaximumSize(240, 16777215);
    }
    else {
        ui->sidebarWidget->setMinimumSize(48, 0);
        ui->sidebarWidget->setMaximumSize(48, 16777215);
    }
}

void MainWindow::setMode(MainWindow::Mode mode) {
    if (mode != _mode) {
        _mode = mode;

        switch (_mode) {
            case HomeMode:
            case ElsewhereMode: {
                ui->pagesWidget->setCurrentIndex(0);
                _loadingImage->stop();
            } break;
            case LoadingMode: {
                ui->pagesWidget->setCurrentIndex(1);
                _loadingImage->start();
            } break;
            case InvalidMode: {
                // Do nothing?
            } break;
        }
        updateGeometry();
        repaint();
        qApp->processEvents();
    }
}

void MainWindow::on_toggleButton_toggled(bool checked) {
    setMenuExpanded(checked);
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    if (obj != ui->lineEdit) return false;

    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* e = dynamic_cast<QKeyEvent*>(event);
        if (e) {
            switch (e->key()) {
                case Qt::Key_Up: {
                    if (_currentIndex < 100 && (_currentIndex + 1) < _scriptHistory.size()) {
                        _currentIndex++;
                        ui->lineEdit->setText(_scriptHistory.at(_currentIndex));
                    }
                } break;
                case Qt::Key_Down: {
                    if (_currentIndex >= 0) {
                        _currentIndex--;
                        ui->lineEdit->setText(_currentIndex < 0 ? QString() : _scriptHistory.at(_currentIndex));
                    }
                } break;
                default:
                    return false;
            }
            return true;
        }
    }
    return false;
}
