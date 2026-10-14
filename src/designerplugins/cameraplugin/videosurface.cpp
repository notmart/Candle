#include "videosurface.h"

VideoSurface::VideoSurface(QObject *parent) : QObject(parent),
                                              m_sink(new QVideoSink(this))
{
    connect(m_sink, &QVideoSink::videoFrameChanged, this, [this](const QVideoFrame &frame) {
        if (!frame.isValid())
            return;

        QImage image = frame.toImage();

#ifdef Q_OS_WIN
        emit frameReady(image.mirrored(false, true));
#else
        emit frameReady(image);
#endif
    });
}
