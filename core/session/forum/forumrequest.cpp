#include "forumrequest.h"
#include <QEvent>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QDebug>
#include <QRegularExpression>

const QString POE_EDIT_THREAD = "https://www.pathofexile.com/forum/edit-thread/";
const QString POE_REPLY_THREAD = "https://www.pathofexile.com/forum/post-reply/";

Session::ForumRequest::ForumRequest(QObject* parent, QNetworkAccessManager *manager)
    : QObject(parent)
    , network(manager)
    , timeout(60000)
{
}

void Session::ForumRequest::timerEvent(QTimerEvent *event) {
    int index = event->timerId();
    for (ForumSubmission* submission : submissions) {
        if (submission->timerId == index) {
            // Uh oh, a submission timed out...
            if (submission->reply) {
                submission->reply->abort();
            }
            break;
        }
    }
    event->accept();
}

void Session::ForumRequest::beginRequest(ForumSubmission* submission) {
    if (submissions.contains(submission->threadId) || submission->threadId.isEmpty()) {
        emit requestError(submission, "Invalid thread ID, or a request to this thread is already being submitted.");
        return;
    }

    // Generate ID
    submissions.insert(submission->threadId, submission);
    submission->timerId = startTimer(getTimeout());

    submission->request = QNetworkRequest(QUrl(POE_EDIT_THREAD + submission->threadId));
    Session::Global()->setAttribute(&submission->request, Session::ForumSubmissionData, QVariant::fromValue<ForumSubmission*>(submission));

    QNetworkReply *fetched = network->get(submission->request);
    submission->state = FORUM_SUBMISSION_STARTED;
    submission->reply = fetched;
    connect(fetched, SIGNAL(finished()), this, SLOT(onSubmissionPageFinished()));
}

ForumSubmission* Session::ForumRequest::extractResponse(QNetworkReply *reply) {
    ForumSubmission* submission = Session::Global()->getAttribute(reply->request(), Session::ForumSubmissionData).value<ForumSubmission*>();
    QString error;

    if (!submission) return 0;

    if (reply->error() != QNetworkReply::NoError) {
        error = "A network error occurred: " + reply->errorString();
        if (reply->error() == QNetworkReply::OperationCanceledError) {
            error = "Timed out while submitting to the server";
        }
    }
    else {
        QByteArray bytes = reply->readAll();
        switch (submission->state) {
        case FORUM_SUBMISSION_STARTED: {
            {
                static QString TokenPart = "name=\"forum_thread\" value=\"";
                int index = bytes.indexOf(TokenPart);
                QString hash = index != -1 ? bytes.mid(index + TokenPart.length(), 32) : "";
                if (hash.isEmpty()) {
                    error = "Could not extract forum token";
                    break;
                }
                submission->data.insert("forum_thread", hash);
                qDebug() << "Extracted Token: " << hash;
            }
            {
                static QString FirstPart = QRegularExpression::escape("<input type=\"text\" name=\"title\" id=\"title\" value=\"");
                static QString LastPart  = QRegularExpression::escape("\" class=\"textInput\">");
                static QRegularExpression expr(QString("%1(?<title>.+)%2").arg(FirstPart).arg(LastPart));

                QRegularExpressionMatch match = expr.match(bytes);
                QString title = match.hasMatch() ? match.captured("title") : "";
                if (title.isEmpty()) {
                    error = "Could not extract title from forum thread";
                    break;
                }
                submission->data.insert("forum_title", title);
                qDebug() << "Extracted Title: " << title;
            }
        } break;
        case FORUM_SUBMISSION_SUBMITTING: {
            static QString FirstPart = QRegularExpression::escape("<ul class=\"errors\"><li>");
            static QString LastPart  = QRegularExpression::escape("</li></ul>");
            static QRegularExpression expr(QString("%1(?<error>.+)%2").arg(FirstPart).arg(LastPart));

            QRegularExpressionMatch match = expr.match(bytes);
            QString forumError = match.hasMatch() ? match.captured("error") : "";
            if (!forumError.isEmpty()) {
                error = "The forum responded with an error: " + forumError;
                break;
            }
        } break;
        case FORUM_SUBMISSION_IDLE:
        case FORUM_SUBMISSION_FINISHED:
        default:
            error = "Internal error";
        }
    }

    if (submission->timerId != -1) {
        killTimer(submission->timerId);
        submission->timerId = -1;
    }

    reply->disconnect();
    reply->deleteLater();

    if (!error.isEmpty()) {
        if (submission)
            removeRequest(submission);
        emit requestError(submission, error);
        return 0;
    }
    return submission;
}

void Session::ForumRequest::onSubmissionPageFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(QObject::sender());
    ForumSubmission* submission = extractResponse(reply);
    if (!submission) return;

    emit requestReady(submission);

    submitRequest(submission);
}

void Session::ForumRequest::submitRequest(ForumSubmission* submission) {
    QUrlQuery query;
    QString encodedData = submission->data.value("content").toString();
    query.addQueryItem("forum_thread", submission->data.value("forum_thread").toString());
    query.addQueryItem("title", submission->data.value("forum_title").toString());
    query.addQueryItem("content", encodedData.replace("+", "%2b"));
    query.addQueryItem("submit", "Submit");

    // Restart the timer
    submission->timerId = startTimer(getTimeout());

    submission->request = QNetworkRequest(QUrl(POE_EDIT_THREAD + submission->threadId));
    Session::Global()->setAttribute(&submission->request, Session::ForumSubmissionData, QVariant::fromValue<ForumSubmission*>(submission));
    submission->request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply *submitted = network->post(submission->request, query.query().toUtf8());
    submission->state = FORUM_SUBMISSION_SUBMITTING;
    submission->reply = submitted;
    connect(submitted, SIGNAL(finished()), this, SLOT(onSubmitted()));
}


void Session::ForumRequest::removeRequest(ForumSubmission *submission) {
    if (submission->timerId != -1) {
        killTimer(submission->timerId);
        submission->timerId = -1;
    }
    submissions.remove(submission->threadId);
}

void Session::ForumRequest::onSubmitted() {
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(QObject::sender());
    ForumSubmission* submission = extractResponse(reply);
    if (!submission) return;

    submission->state = FORUM_SUBMISSION_FINISHED;
    submission->reply = 0;
    removeRequest(submission);
    emit requestFinished(submission);
}
