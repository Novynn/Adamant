#ifndef PLUGINITEMDELEGATE_H
#define PLUGINITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <ui/widgets/pluginitem.h>
#include <adamantplugininfo.h>
#include <QDebug>
#include <QApplication>

class PluginItemDelegate : public QStyledItemDelegate
{
public:
    PluginItemDelegate()
        : QStyledItemDelegate()
        , _item(new PluginItem()){

    }

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        setItemStateFromIndex(option, index);
        painter->save();
        _item->resize(option.rect.size());
        painter->translate(option.rect.topLeft());
        _item->render(painter);
        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
        setItemStateFromIndex(option, index);
        return _item->size();
    }
private:
    void setItemStateFromIndex(const QStyleOptionViewItem &option, const QModelIndex &index) const {
        _item->setSelected(option.state & QStyle::State_Selected);

        AdamantPluginInfo* container = index.data(Qt::UserRole).value<AdamantPluginInfo*>();
        if (container) {
            _item->setText(container->name);
            _item->setState(container->state);
            _item->setVersionString(container->metaData.value("version").toString());
        }
        else {
            // Core Package
            _item->setText(index.data(Qt::DisplayRole).toString());
            _item->setState(PluginState::Unknown);
            _item->setVersionString(QApplication::applicationVersion());
        }
    }

    PluginItem* _item;
};

#endif // PLUGINITEMDELEGATE_H
