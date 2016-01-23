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
public slots:
    QDir CacheDir() const;
    void GetImage(QString path);
    QString GenerateFileName(const QString &path);
signals:
    void OnImage(const QString &path, QImage image);
private slots:
    void OnImageResult();
private:
    void Save(const QString &path, const QImage &image);
    QNetworkAccessManager* _manager;

    QHash<QString, QImage> _cache;
};
Q_DECLARE_METATYPE(ImageCache*)

#endif // IMAGECACHE_H
