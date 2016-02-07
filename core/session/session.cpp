#include "session.h"
#include "core.h"
#include "sessionrequest.h"

Session::Request* Session::_globalRequest = nullptr;
CoreService* Session::_core = nullptr;

void Session::SetCoreService(CoreService *core) {
    _core = core;
}

void Session::LogError(const QString &error) {
    emit _core->message(error, QtMsgType::QtWarningMsg);
}

Session::Request *Session::Global() {
    if (_globalRequest == nullptr) {
        _globalRequest = NewRequest(_core);
    }
    return _globalRequest;
}

Session::Request *Session::NewRequest(QObject *parent) {
    auto request = new Request(parent);
    return request;
}
