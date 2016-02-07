#ifndef LOGGINGSYSTEM_H
#define LOGGINGSYSTEM_H

#include <QObject>
#include <QDebug>

class LoggingSystem : public QObject
{
    Q_OBJECT
public:
    static LoggingSystem* Get();

public slots:
    static void qtMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    void message(const QString &message, QtMsgType type);
signals:
    void onMessage(const QString &message, QtMsgType type);
private:
    explicit LoggingSystem();
    static LoggingSystem* _instance;
};

#endif // LOGGINGSYSTEM_H
