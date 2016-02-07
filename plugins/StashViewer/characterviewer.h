#ifndef CHARACTERVIEWER_H
#define CHARACTERVIEWER_H

#include <QWidget>

namespace Ui {
class CharacterViewer;
}

class CharacterViewer : public QWidget
{
    Q_OBJECT

public:
    explicit CharacterViewer(QWidget *parent = 0);
    ~CharacterViewer();

    void setCharacters(QStringList names);

private:
    Ui::CharacterViewer *ui;
};

#endif // CHARACTERVIEWER_H
