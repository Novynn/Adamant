#include "characterviewer.h"
#include "ui_characterviewer.h"

CharacterViewer::CharacterViewer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CharacterViewer) {
    ui->setupUi(this);

    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 4);
}

CharacterViewer::~CharacterViewer() {
    delete ui;
}

void CharacterViewer::setCharacters(QStringList names) {
    ui->listWidget->clear();
    ui->listWidget->addItems(names);
}
