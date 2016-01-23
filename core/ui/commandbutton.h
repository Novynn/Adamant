#ifndef COMMANDBUTTON_H
#define COMMANDBUTTON_H

#include <QCommandLinkButton>

class CommandButton : public QCommandLinkButton
{
public:
    explicit CommandButton(QWidget *parent=0) : QCommandLinkButton(parent) {}
    explicit CommandButton(const QString &text, QWidget *parent=0) : QCommandLinkButton(text, parent) {}
    explicit CommandButton(const QString &text, const QString &description, QWidget *parent=0) : QCommandLinkButton(text, description, parent) {}

    void setIconOnly(bool b) {
        _iconOnly = b;
        if (_iconOnly) {
            _text = text();
            _description = description();

            setText(QString());
            setDescription(QString());
        }
        else {
            if (text().isEmpty()) setText(_text);
            if (description().isEmpty()) setDescription(_description);
        }
    }
private:
    bool _iconOnly;
    QString _text;
    QString _description;
};

#endif // COMMANDBUTTON_H
