#ifndef CAMERAGETDATAWORKER_H
#define CAMERAGETDATAWORKER_H

#include <QObject>
#include <QVariant>
#include <QSharedPointer>
#include "DeviceControl/cameracontrol.h"

class CameraGetDataWorker : public QObject
{
    Q_OBJECT
public:
    explicit CameraGetDataWorker(QVector<QSharedPointer<CameraControl> > &camobj, QObject *parent = nullptr);

signals:

public slots:
    void getAllImages();

private:
    QVector<QSharedPointer<CameraControl>> m_cameras;
};

#endif // CAMERAGETDATAWORKER_H
