#include "adamant.h"
#include <QApplication>
#include "loggingsystem.h"
#include <pluginmanager.h>

#include <core.h>

Adamant::Adamant(QObject *parent)
    : QObject(parent)
    , _loggingSystem(LoggingSystem::Get())
    , _core(new CoreService) {
    connect(_core, &CoreService::message, _loggingSystem, &LoggingSystem::message);
    connect(_loggingSystem, &LoggingSystem::onMessage, _core, &CoreService::loggedMessage);
}

Adamant::~Adamant() {
    delete _core;
}

bool Adamant::requestApplicationExit() const {
    // Invoke the exit rather than straight out quit, to allow other actions to complete.
    return QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
}

bool Adamant::start() {
    return _core->load();
}
