#include "adamant.h"
#include <QApplication>
#include "loggingsystem.h"
#include <pluginmanager.h>

#include <core.h>

Adamant::Adamant(QObject *parent)
    : QObject(parent)
    , _loggingSystem(LoggingSystem::Get())
    , _core(new CoreService) {
    connect(_core, &CoreService::Message, _loggingSystem, &LoggingSystem::Message);
    connect(_loggingSystem, &LoggingSystem::OnMessage, _core, &CoreService::LoggedMessage);
}

bool Adamant::RequestApplicationExit() const {
    // Invoke the exit rather than straight out quit, to allow other actions to complete.
    return QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
}

void Adamant::start() {
    _core->Load();
}
