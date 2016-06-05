#ifndef FORUMREQUEST_H
#define FORUMREQUEST_H

#include <core_global.h>
#include <session/sessionrequest.h>

enum ForumSubmissionState {
    FORUM_SUBMISSION_IDLE,
    FORUM_SUBMISSION_STARTED,
    FORUM_SUBMISSION_SUBMITTING,
    FORUM_SUBMISSION_FINISHED
};

struct ForumSubmission {
    QString threadId;
    ForumSubmissionState state;
    int timerId;
    QVariantHash data;
    QNetworkRequest request;
    QNetworkReply* reply;
};

class CORE_EXTERN Session::ForumRequest : public QObject
{
    Q_OBJECT
public:
    explicit ForumRequest(QObject* parent, QNetworkAccessManager* manager);

    bool isSubmitting(const QString &threadId) {
        return submissions.contains(threadId) &&
               submissions.value(threadId)->state != FORUM_SUBMISSION_IDLE &&
               submissions.value(threadId)->state != FORUM_SUBMISSION_FINISHED;
    }

    int count() {
        int count = 0;
        for (ForumSubmission* submission : submissions) {
            if (isSubmitting(submission->threadId))
                count++;
        }
        return count;
    }

    void setTimeout(int t) { timeout = t; }
    int getTimeout() { return timeout; }

protected:
    void timerEvent(QTimerEvent *event);
signals:
    void requestError(const ForumSubmission* submission, const QString &error);
    void requestReady(const ForumSubmission* submission);
    void requestFinished(const ForumSubmission* submission);
public slots:
    void beginRequest(ForumSubmission* submission);
private slots:
    void onSubmissionPageFinished();
    void onSubmitted();
private:
    ForumSubmission* extractResponse(QNetworkReply *reply);
    void submitRequest(ForumSubmission* submission);
    void removeRequest(ForumSubmission* submission);
    QNetworkAccessManager* network;
    QHash<QString, ForumSubmission*> submissions;

    int timeout;
};

Q_DECLARE_METATYPE(ForumSubmission*)

#endif // FORUMREQUEST_H
