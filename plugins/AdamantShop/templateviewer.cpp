#include "templateviewer.h"
#include "ui_templateviewer.h"
#include <adamantshopplugin.h>

TemplateViewer::TemplateViewer(AdamantShopPlugin* plugin, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TemplateViewer)
    , _template() {
    ui->setupUi(this);
}

TemplateViewer::~TemplateViewer()
{
    delete ui;
}

void TemplateViewer::on_textEdit_textChanged() {
//    _template.setSectionTemplate(ShopTemplate::Body, ui->textEdit->toPlainText());
//    ui->textBrowser->clear();
//    if (_template.isValid()) {
//        ui->textBrowser->setTextColor(ui->textEdit->palette().color(QPalette::Text));

////        _template.render({ShopTemplate::Header, ShopTemplate::Body, ShopTemplate::Footer});
//    }
//    else {
//        ui->textBrowser->setTextColor(Qt::red);
//        ui->textBrowser->setPlainText(_template.errorMessage(ShopTemplate::Body));
//    }

}
