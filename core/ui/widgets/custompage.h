#ifndef CUSTOMPAGE_H
#define CUSTOMPAGE_H

#include <QObject>
#include <QUuid>

class CommandButton;
class QShortcut;

class CustomPage : public QObject
{
    Q_OBJECT
public:
    CustomPage(QUuid id, const QIcon& icon, const QString& title, const QString& description, QWidget* widget, QObject* owner = 0);

    QUuid id() const {
        return _id;
    }

    CommandButton* button() const {
        return _button;
    }

    QWidget* widget() const {
        return _widget;
    }

    QObject* owner() const {
        return _owner;
    }

private slots:
    void activate();
signals:
    void activated(QUuid id);
private:
    QUuid _id;
    CommandButton* _button;
    QWidget* _widget;
    QObject* _owner;
};

#endif // CUSTOMPAGE_H
