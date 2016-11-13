#ifndef SHOPVIEWER_H
#define SHOPVIEWER_H

#include <QDir>
#include <QMenu>
#include <QWidget>

namespace Ui {
class ShopViewer;
}

class AdamantShopPlugin;
class Shop;
class QListWidgetItem;
class StashViewer;
class NewShopDialog;
class ForumSubmission;
class QPushButton;

class ShopViewer : public QWidget
{
    Q_OBJECT

public:
    explicit ShopViewer(AdamantShopPlugin* plugin, StashViewer* viewer, QWidget *parent = 0);
    ~ShopViewer();

    void setLeagues(const QStringList &leagues);
    void setupStashIntegration();
    void addShop(const Shop* shop);
    void updateShopListItem(const Shop* shop);
    void showShop(const Shop* shop);
    void removeShop(Shop* shop);
private slots:
    void on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_viewItemsButton_clicked();

    void on_listWidget_customContextMenuRequested(const QPoint &pos);

    void on_addThreadButton_clicked();
    void on_updateButton_clicked();

private:
    AdamantShopPlugin* _plugin;
    StashViewer* _stashViewer;
    QMap<const Shop*, QListWidgetItem*> _shopListItems;
    Shop* _currentShop;
    QList<const ForumSubmission*> _submissions;
    Ui::ShopViewer *ui;
    QStringList _leagues;

    QPushButton* _tabWidePriceButton;
};

#endif // SHOPVIEWER_H
