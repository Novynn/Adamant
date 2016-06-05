#include "loggingsystem.h"

LoggingSystem* LoggingSystem::_instance = 0;

LoggingSystem::LoggingSystem()
    : QObject()
{
    qInstallMessageHandler(LoggingSystem::qtMessage);
    qSetMessagePattern("[%{time h:mm:ss.zzz}] %{if-debug}%{file}:%{line} - %{endif}%{message}");
}


LoggingSystem *LoggingSystem::Get() {
    if (!_instance) {
        _instance = new LoggingSystem();
    }
    return _instance;
}

void LoggingSystem::qtMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
#ifndef QT_DEBUG
    // Redirect debug for now
    if (type == QtDebugMsg) type = QtInfoMsg;
#endif
    QString formatted = qFormatLogMessage(type, context, msg);
    LoggingSystem* t = Get();
    t->message(formatted, type);
}

void LoggingSystem::message(const QString &message, QtMsgType type) {

    fprintf(stdout, "%s\n", qPrintable(message));
    emit onMessage(message, type);
}
