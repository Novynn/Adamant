#ifndef SKILLTREEVIEWER_H
#define SKILLTREEVIEWER_H

#include <QWidget>

namespace Ui {
class SkillTreeViewer;
}

class AdamantSkillTreePlugin;

class SkillTreeViewer : public QWidget
{
    Q_OBJECT

public:
    explicit SkillTreeViewer(AdamantSkillTreePlugin* plugin = 0);
    ~SkillTreeViewer();
protected:
    virtual void wheelEvent(QWheelEvent* event);
    virtual bool eventFilter(QObject *watched, QEvent *event);
private:
    Ui::SkillTreeViewer *ui;
    AdamantSkillTreePlugin* _plugin;
};

#endif // SKILLTREEVIEWER_H
