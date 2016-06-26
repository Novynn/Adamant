#ifndef TEMPLATEVIEWER_H
#define TEMPLATEVIEWER_H

#include <QWidget>
#include <shoptemplate.h>

class AdamantShopPlugin;

namespace Ui {
class TemplateViewer;
}

class TemplateViewer : public QWidget
{
    Q_OBJECT

public:
    explicit TemplateViewer(AdamantShopPlugin* plugin, QWidget *parent = 0);
    ~TemplateViewer();

private slots:
    void on_textEdit_textChanged();

private:
    Ui::TemplateViewer *ui;
    ShopTemplate _template;
    std::function<void(const QString&)> _handler;
};

#endif // TEMPLATEVIEWER_H
