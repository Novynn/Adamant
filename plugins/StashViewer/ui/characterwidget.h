#ifndef CHARACTERWIDGET_H
#define CHARACTERWIDGET_H

#include <QWidget>
#include <characterviewer.h>

namespace Ui {
class CharacterWidget;
}

class CharacterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CharacterWidget(QWidget *parent, const Character &character);
    ~CharacterWidget();

    QString getCharacterName();

    QString getCharacterLeague();
private:
    Ui::CharacterWidget *ui;
};

#endif // CHARACTERWIDGET_H
