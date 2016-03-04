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
    void setLeagues(QStringList leagues);
private slots:
    void on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);

    void on_filterBox_currentIndexChanged(const QString &str);

private:
    Ui::CharacterViewer *ui;

    QMap<QString, CharacterItemLocation*> _locations;
    QMap<QString, QGraphicsRectItem*> _characterViews;
    StashScene* _scene;

    ImageCache* _imageCache;
    GraphicItemFactory* _factory;
    QThread* _factoryThread;
    bool filterItem(const QString& league);
};
Q_DECLARE_METATYPE(Character)

#endif // CHARACTERVIEWER_H
