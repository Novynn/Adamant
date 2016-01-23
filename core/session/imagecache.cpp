#include "imagecache.h"

#include <QDir>
#include <QNetworkReply>
#include <QNetworkRequest>
#include "core.h"

ImageCache::ImageCache(QObject *parent)
    : QObject(parent)
    , _manager(new QNetworkAccessManager(parent))
{
}

QDir ImageCache::CacheDir() const {
    QDir cacheDir = CoreService::dataPath();
    if (!cacheDir.cd("cache")) {
        cacheDir.mkdir("cache");
        cacheDir.cd("cache");
    }
    return cacheDir;
}

void ImageCache::GetImage(QString path) {
    QString file = GenerateFileName(path);
    QString local = CacheDir().absoluteFilePath(file);

    // Check memory cache
    if (_cache.contains(file)) {
        emit OnImage(file, _cache.value(file));
        return;
    }
    // Check system cache
    else if (QFile::exists(local)) {
        QImage image(local, "png");
        if (!image.isNull()) {
            emit OnImage(file, image);
            return;
        }
    }
    else {
        QNetworkRequest request = QNetworkRequest(path);
        request.setAttribute(QNetworkRequest::User, file);
        QNetworkReply *r = _manager->get(request);
        connect(r, &QNetworkReply::finished, this, &ImageCache::OnImageResult);
    }
}

QString ImageCache::GenerateFileName(const QString &path) {
    return QString(QCryptographicHash::hash(path.toLatin1(), QCryptographicHash::Md5).toHex()).append(".png");
}

void ImageCache::Save(const QString &file, const QImage &image) {
    // Keep only 1000 images in cache
    // TODO(rory): Allow this to be variable?
    while (_cache.size() > 1000) _cache.remove(_cache.keys().first());
    _cache.insert(file, image);
    image.save(CacheDir().absoluteFilePath(file));
}

void ImageCache::OnImageResult() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
    if (reply->error()) {
        qDebug() << "Network error in " << __FUNCTION__ << ": " << reply->errorString();
    }
    else {
        QNetworkRequest request = reply->request();
        const QString file = request.attribute(QNetworkRequest::User).toString();
        QImage image = QImage::fromData(reply->readAll(), "png");
        if (!image.isNull()) {
            Save(file, image);
            emit OnImage(file, image);
        }
    }
    reply->deleteLater();
}

