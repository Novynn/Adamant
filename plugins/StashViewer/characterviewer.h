#ifndef CHARACTERVIEWER_H
#define CHARACTERVIEWER_H

#include <QGraphicsPixmapItem>
#include <QThread>
#include <QWidget>

#include <items/characteritemlocation.h>

namespace Ui {
class CharacterViewer;
}

class StashScene;
class GraphicItemFactory;
class ImageCache;
class QListWidgetItem;

struct Character {
    QString name;
    QString league;
    QString classType;
    int level;
};

class CharacterViewer : public QWidget
{
    Q_OBJECT

public:
    explicit CharacterViewer(QWidget *parent = 0);
    ~CharacterViewer();

    void setCharacters(QList<Character> characters);
    void setCharacterItems(const QString &character, CharacterItemLocation* location);
    void OnImage(const QString& path, QImage image);
private slots:
    void on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

private:
    Ui::CharacterViewer *ui;

    QMap<QString, CharacterItemLocation*> _locations;
    QMap<QString, QGraphicsRectItem*> _characterViews;
    StashScene* _scene;

    ImageCache* _imageCache;
    GraphicItemFactory* _factory;
    QThread* _factoryThread;
};

#endif // CHARACTERVIEWER_H
