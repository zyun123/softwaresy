#include <QSharedPointer>
#include <QThread>
#include <QDateTime>
#include <opencv2/opencv.hpp>
#include "global.h"
#include "cameragetdataworker.h"

CameraGetDataWorker::CameraGetDataWorker(QVector<QSharedPointer<CameraControl> > &camobj, QObject *parent) : QObject(parent)
{
   for(int i = 0; i < camobj.size(); i++)
       m_cameras.append(camobj[i]);
}

void CameraGetDataWorker::getAllImages()
{
    while(1)
    {
        if(QThread::currentThread()->isInterruptionRequested())
            return;
        for(int i = 0; i < m_cameras.size(); i++)
            m_cameras[i]->getImageFromshm();
        QThread::msleep(80);
    }
}
