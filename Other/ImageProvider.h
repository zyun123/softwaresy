#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QImage>
#include <QSize>
#include <QColor>
#include <QObject>
class ImageProvider: public QQuickImageProvider

{
public:
    ImageProvider();

       QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
       QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);

       QImage img;

};

#endif // IMAGEPROVIDER_H
