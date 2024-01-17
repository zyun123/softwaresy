#include "frameprovider.h"
#include "DeviceControl/cameracontrol.h"

FrameProvider::FrameProvider(QObject *parent) : QObject(parent)
{

}

void FrameProvider::setVideoSurface(QAbstractVideoSurface *surface)
{
    if (m_surface && m_surface != surface  && m_surface->isActive()) {
        m_surface->stop();
    }

    m_surface = surface;
    setFormat(IMAGE_WIDTH, IMAGE_HEIGHT, QVideoFrame::Format_ARGB32);

    if (m_surface && m_format.isValid())
    {
        m_format = m_surface->nearestFormat(m_format);
        m_surface->start(m_format);

    }
}

void FrameProvider::setFormat(int width, int heigth, QVideoFrame::PixelFormat format)
{
    QSize size(width, heigth);
    QVideoSurfaceFormat surformat(size, format);
    m_format = surformat;

    if (m_surface)
    {
        if (m_surface->isActive())
        {
            m_surface->stop();
        }
        m_format = m_surface->nearestFormat(m_format);
        m_surface->start(m_format);
    }
}

void FrameProvider::onNewVideoContentReceived(QSharedPointer<QImage> pimg)
{
    if (m_surface)
    {
        auto img = pimg->copy();
        QVideoFrame fr(img);
        m_surface->present(fr);
    }
}
