#include "imagecache.h"

#include <QDir>
#include <QNetworkReply>
#include <QNetworkRequest>
#include "core.h"

ImageCache::ImageCache(QObject *parent)
    : QObject(parent)
    , _manager(new QNetworkAccessManager(parent))
{
    connect(_manager, &QNetworkAccessManager::sslErrors, this, [this] (QNetworkReply* reply, const QList<QSslError> &errors) {
        Q_UNUSED(errors)
        reply->ignoreSslErrors();
    });
}

QDir ImageCache::cacheDir() const {
    QDir cacheDir = CoreService::cachePath();
    if (!cacheDir.cd("image_cache")) {
        cacheDir.mkdir("image_cache");
        cacheDir.cd("image_cache");
    }
    return cacheDir;
}

bool ImageCache::hasLocalImage(QString path) {
    QString file = generateFileName(path);
    QString local = cacheDir().absoluteFilePath(file);

    // Check memory cache
    if (_cache.contains(file)) {
        return true;
    }
    // Check system cache
    else if (QFile::exists(local)) {
        QImage image(local, "png");
        if (!image.isNull()) {
            return true;
        }
    }
    return false;
}

QImage ImageCache::getImage(QString path) {
    QString file = generateFileName(path);
    QString local = cacheDir().absoluteFilePath(file);

    // Check memory cache
    if (_cache.contains(file)) {
        return _cache.value(file);
    }
    // Check system cache
    else if (QFile::exists(local)) {
        QImage image(local, "png");
        if (!image.isNull()) {
            return image;
        }
    }
    fetchImage(path);
    return QImage();
}

void ImageCache::fetchImage(QString path) {
    QString file = generateFileName(path);
    QNetworkRequest request = QNetworkRequest(path);
    request.setAttribute(QNetworkRequest::User, file);
    QNetworkReply *r = _manager->get(request);
    connect(r, &QNetworkReply::finished, this, &ImageCache::onImageResult);
}

QString ImageCache::generateFileName(const QString &path) {
    return QString(QCryptographicHash::hash(path.toLatin1(), QCryptographicHash::Md5).toHex()).append(".png");
}

void ImageCache::save(const QString &file, const QImage &image) {
    // Keep only 1000 images in cache
    // TODO(rory): Allow this to be variable?
    while (_cache.size() > 1000) _cache.remove(_cache.keys().first());
    _cache.insert(file, image);
    image.save(cacheDir().absoluteFilePath(file));
}

void ImageCache::onImageResult() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
    if (reply->error()) {
        qDebug() << "Network error in " << __FUNCTION__ << ": " << reply->errorString();
    }
    else {
        QNetworkRequest request = reply->request();
        const QString file = request.attribute(QNetworkRequest::User).toString();
        QImage image = QImage::fromData(reply->readAll(), "png");
        if (!image.isNull()) {
            save(file, image);
            emit onImage(file, image);
        }
    }
    reply->deleteLater();
}

