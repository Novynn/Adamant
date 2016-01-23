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
        SetItemStateFromIndex(option, index);
        painter->save();
        _item->resize(option.rect.size());
        painter->translate(option.rect.topLeft());
        _item->render(painter);
        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
        SetItemStateFromIndex(option, index);
        return _item->size();
    }
private:
    void SetItemStateFromIndex(const QStyleOptionViewItem &option, const QModelIndex &index) const {
        _item->SetSelected(option.state & QStyle::State_Selected);

        AdamantPluginInfo* container = index.data(Qt::UserRole).value<AdamantPluginInfo*>();
        if (container) {
            _item->SetText(container->name);
            _item->SetState(container->state);
            _item->SetVersionString(container->metaData.value("version").toString());
        }
        else {
            // Core Package
            _item->SetText(index.data(Qt::DisplayRole).toString());
            _item->SetState(PluginState::Unknown);
            _item->SetVersionString(QApplication::applicationVersion());
        }
    }

    PluginItem* _item;
};

#endif // PLUGINITEMDELEGATE_H
