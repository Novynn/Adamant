#include "loggingsystem.h"

LoggingSystem* LoggingSystem::_instance = 0;

LoggingSystem *LoggingSystem::Get() {
    if (!_instance) {
        _instance = new LoggingSystem();
    }
    return _instance;
}

void LoggingSystem::QtMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QString formatted = qFormatLogMessage(type, context, msg);
    LoggingSystem* t = Get();
    t->Message(formatted, type);
}

void LoggingSystem::Message(const QString &message, QtMsgType type) {

    fprintf(stdout, "%s\n", qPrintable(message));
    emit OnMessage(message, type);
}

LoggingSystem::LoggingSystem()
    : QObject()
{
    qInstallMessageHandler(LoggingSystem::QtMessage);
    qSetMessagePattern("[%{time h:mm:ss.zzz}] %{file}:%{line} - %{message}");
}

