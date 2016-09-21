#include "characterwidget.h"
#include "ui_characterwidget.h"

CharacterWidget::CharacterWidget(QWidget *parent, const Character& character) :
    QWidget(parent),
    ui(new Ui::CharacterWidget) {
    ui->setupUi(this);

    ui->nameLabel->setText(character.name);
    ui->descLabel->setText(QString("Level %1 %2").arg(character.level).arg(character.classType));
    ui->leagueLabel->setText(character.league);
    QImage image(":/character/" + character.classType.toLower() + ".png");
    ui->characterTypeImage->setPixmap(QPixmap::fromImage(image));
}

CharacterWidget::~CharacterWidget() {
    delete ui;
}

QString CharacterWidget::getCharacterName() {
    return ui->nameLabel->text();
}

QString CharacterWidget::getCharacterLeague() {
    return ui->leagueLabel->text();
}
