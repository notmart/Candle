#pragma once

#include <QObject>
#include <QImage>
#include <QVideoSink>
#include <QVideoFrame>

class VideoSurface : public QObject
{
    Q_OBJECT

public:
    VideoSurface(QObject *parent = nullptr);

    QVideoSink *sink() const { return m_sink; }

signals:
    void frameReady(const QImage &image);

private:
    QVideoSink *m_sink;
};
