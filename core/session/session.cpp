#include "session.h"
#include "core.h"
#include "sessionrequest.h"
#include "forum/forumrequest.h"

Session::Request* Session::_globalRequest = nullptr;
Session::ForumRequest* Session::_forumRequest = nullptr;
CoreService* Session::_core = nullptr;

void Session::SetCoreService(CoreService *core) {
    _core = core;
}

void Session::LogError(const QString &error) {
    emit _core->message(error, QtMsgType::QtWarningMsg);
}

Session::Request* Session::Global() {
    if (_globalRequest == nullptr) {
        _globalRequest = NewRequest(_core);
    }
    return _globalRequest;
}

Session::ForumRequest* Session::Forum() {
    if (_forumRequest == nullptr) {
        _forumRequest = new ForumRequest(_core, Global()->_manager);
    }
    return _forumRequest;
}

Session::Request *Session::NewRequest(QObject *parent) {
    auto request = new Request(parent);
    return request;
}
