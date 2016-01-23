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

    void start();

    Q_INVOKABLE CoreService* Core() {
        return _core;
    }
public slots:
    bool RequestApplicationExit() const;
private:
    LoggingSystem* _loggingSystem;
    CoreService* _core;

};

#endif // ADAMANT_H
