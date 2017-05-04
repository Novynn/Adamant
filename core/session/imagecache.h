#ifndef IMAGECACHE_H
#define IMAGECACHE_H

#include <core_global.h>
#include <QNetworkAccessManager>
#include <QObject>
#include <QImage>
#include <QDir>

class CORE_EXTERN ImageCache : public QObject
{
    Q_OBJECT
public:
    explicit ImageCache(QObject *parent = 0);
    bool hasLocalImage(QString path);
public slots:
    QDir cacheDir() const;
    void fetchImage(QString path, QVariant data = QVariant());
    QImage getImage(QString path, QVariant data = QVariant());
    QString generateFileName(const QString &path);
signals:
    void onImage(const QString &path, const QImage image, const QVariant data = QVariant());
private slots:
    void onImageResult();
private:
    void save(const QString &path, const QImage &image);
    QNetworkAccessManager* _manager;

    QHash<QString, QImage> _cache;
};
Q_DECLARE_METATYPE(ImageCache*)

#endif // IMAGECACHE_H
