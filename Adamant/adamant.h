#ifndef ADAMANT_H
#define ADAMANT_H

#include <QObject>

class LoggingSystem;
class CoreService;

class Adamant : public QObject
{
    Q_OBJECT
public:
    explicit Adamant(QObject *parent = 0);
    ~Adamant();

    void start();

    Q_INVOKABLE CoreService* core() {
        return _core;
    }
public slots:
    bool requestApplicationExit() const;
private:
    LoggingSystem* _loggingSystem;
    CoreService* _core;

};

#endif // ADAMANT_H
