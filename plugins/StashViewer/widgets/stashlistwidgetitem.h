#ifndef STASHLISTWIDGETITEM_H
#define STASHLISTWIDGETITEM_H

#include <QListWidget>
#include <QWidget>

// TODO(rory): Remove this when we switch to an actual data model
class StashListWidgetItem : public QListWidgetItem
{
public:
    explicit StashListWidgetItem(const QString &text, QListWidget *view = 0, int index = 0)
        : QListWidgetItem(text, view) {
        const int offset = 204; // Some random number
        setData((int)Qt::UserRole + offset, index);
    }

    bool operator<(const QListWidgetItem& other) const {
        const int offset = 204; // Some random number
        return data((int)Qt::UserRole + offset) < other.data((int)Qt::UserRole + offset);
    }
};

#endif // STASHLISTWIDGETITEM_H
