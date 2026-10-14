#include "camerawidget.h"

#include <QCamera>
#include <QMediaDevices>
#include <QMediaCaptureSession>
#include <QScrollBar>
#include <QMouseEvent>
#include <QPainter>
#include <QVBoxLayout>
#include <QTimer>
#include <QCoreApplication>
#include "videosurface.h"

CameraWidget::CameraWidget(QWidget *parent) : QWidget(parent),
                                              m_camera(nullptr),
                                              m_captureSession(nullptr),
                                              m_videoLabel(nullptr),
                                              m_scrollArea(nullptr),
                                              m_overlay(nullptr),
                                              m_resolution(QSize(1280, 720)),
                                              m_zoom(1.0),
                                              m_mousePos(QPoint(0, 0))
{
    m_videoSurface = new VideoSurface(this);
    connect(m_videoSurface, &VideoSurface::frameReady, this, &CameraWidget::processFrame);

    m_videoLabel = new QLabel(this);
    m_videoLabel->setStyleSheet("background-color: black;");

    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->horizontalScrollBar()->setEnabled(false);
    m_scrollArea->verticalScrollBar()->setEnabled(false);
    m_scrollArea->setWidget(m_videoLabel);
    m_scrollArea->setStyleSheet("QScrollArea { background-color: black; }");

    layout->addWidget(m_scrollArea);
    layout->setContentsMargins(0, 0, 0, 0);

    m_overlay = new Overlay(m_videoLabel);
}

void CameraWidget::setCameraName(QString cameraName)
{
    m_cameraName = cameraName;

    if (!m_cameraName.isEmpty())
    {
        for (const QCameraDevice &cameraDevice : QMediaDevices::videoInputs())
        {
            if (cameraDevice.description() == m_cameraName)
            {
                initCamera(cameraDevice);
                return;
            }
        }
    }

    if (m_camera)
        deleteCamera();
}

QString CameraWidget::cameraName() const
{
    return m_cameraName;
}

QStringList CameraWidget::availableCameras() const
{
    QStringList l;

    for (const QCameraDevice &i : QMediaDevices::videoInputs())
        l << i.description();

    return l;
}

QStringList CameraWidget::availableResolutions() const
{
    if (!m_cameraDevice.isNull())
    {
        QStringList l;
        for (const QCameraFormat &f : m_cameraDevice.videoFormats())
            l.append(QString("%1x%2").arg(f.resolution().width()).arg(f.resolution().height()));
        return l;
    }
    else
    {
        return QStringList();
    }
}

void CameraWidget::setResolution(QVariantList resolution)
{
    if (resolution.size() != 2)
        return;

    m_resolution = QSize(resolution.at(0).toInt(), resolution.at(1).toInt());

    updateSize();
}

QVariantList CameraWidget::resolution() const
{
    return QVariantList() << m_resolution.width() << m_resolution.height();
}

void CameraWidget::setZoom(double zoom)
{
    if (zoom < 1)
        return;

    m_zoom = zoom;

    updateSize();
}

double CameraWidget::zoom() const
{
    return m_zoom;
}

void CameraWidget::setPos(QVariantList pos)
{
    if (pos.size() != 2)
        return;

    m_pos = QPoint(pos.at(0).toInt(), pos.at(1).toInt());

    m_scrollArea->horizontalScrollBar()->setValue(m_pos.x());
    m_scrollArea->verticalScrollBar()->setValue(m_pos.y());

    emit posChanged(pos);
}

QVariantList CameraWidget::pos() const
{
    return QVariantList() << m_pos.x() << m_pos.y();
}

void CameraWidget::setAimPos(QVariantList aimPos)
{
    if (aimPos.size() != 2)
        return;

    if (qIsNaN(aimPos.at(0).toDouble()) || qIsNaN(aimPos.at(1).toDouble()))
        return;

    m_overlay->setAimPosition(QPointF(aimPos.at(0).toDouble(), aimPos.at(1).toDouble()));
    m_overlay->update();

    emit aimPosChanged(aimPos);
}

QVariantList CameraWidget::aimPos() const
{
    QPointF p = m_overlay->aimPosition();

    return QVariantList() << p.x() << p.y();
}

void CameraWidget::setAimSize(int aimSize)
{
    m_overlay->setAimSize(aimSize);

    emit aimSizeChanged(aimSize);
}

int CameraWidget::aimSize() const
{
    return m_overlay->aimSize();
}

void CameraWidget::setAimLineWidth(int aimLineWidth)
{
    m_overlay->setAimLineWidth(aimLineWidth);

    emit aimLineWidthChanged(aimLineWidth);
}

int CameraWidget::aimLineWidth() const
{
    return m_overlay->aimLineWidth();
}

void CameraWidget::setAimColor(int aimColor)
{
    m_overlay->setAimColor(aimColor);

    emit aimColorChanged(aimColor);
}

int CameraWidget::aimColor() const
{
    return m_overlay->aimColor();
}

void CameraWidget::setMirrorHorizontal(bool mirror)
{
    m_mirrorHorizontal = mirror;
}

bool CameraWidget::mirrorHorizontal() const
{
    return m_mirrorHorizontal;
}

void CameraWidget::setMirrorVertical(bool mirror)
{
    m_mirrorVertical = mirror;
}

bool CameraWidget::mirrorVertical() const
{
    return m_mirrorVertical;
}

void CameraWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->buttons() == Qt::LeftButton)
    {
        m_mousePos = e->position().toPoint();
        m_aimPos = m_overlay->aimPosition();
    }

    QWidget::mousePressEvent(e);
}

void CameraWidget::mouseMoveEvent(QMouseEvent *e)
{

    if (e->buttons() == Qt::LeftButton && e->modifiers() == Qt::ShiftModifier)
    {

        QPointF d = QPointF((double)(e->position().toPoint() - m_mousePos).x() / m_overlay->width(),
                            (double)(e->position().toPoint() - m_mousePos).y() / m_overlay->height());

        QPointF o = m_aimPos + d;
        setAimPos(QVariantList() << o.x() << o.y());
    }
    else if (e->buttons() == Qt::LeftButton)
    {
        QPoint d = e->position().toPoint() - m_mousePos;

        setPos(QVariantList() << m_scrollArea->horizontalScrollBar()->value() - d.x()
                              << m_scrollArea->verticalScrollBar()->value() - d.y());

        m_mousePos = e->position().toPoint();
    }

    QWidget::mouseMoveEvent(e);
}

void CameraWidget::wheelEvent(QWheelEvent *e)
{
    double f = e->angleDelta().y() > 0 ? 1.25 : 0.8;

    double prevZoom = m_zoom;
    m_zoom = qBound<double>(1, m_zoom * f, 4);

    QPointF deltaPos = e->position() / prevZoom - m_scrollArea->widget()->pos() / prevZoom;
    QPointF delta = deltaPos * m_zoom - deltaPos * prevZoom;
    QPoint d = delta.toPoint();

    auto horizontalPos = m_scrollArea->horizontalScrollBar()->value();
    auto verticalPos = m_scrollArea->verticalScrollBar()->value();

    updateSize();

    setPos(QVariantList() << horizontalPos + d.x() << verticalPos + d.y());
    setAimPos(QVariantList() << m_overlay->aimPosition().x() << m_overlay->aimPosition().y());

    emit zoomChanged(m_zoom);
}

void CameraWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    updateSize();

    setPos(QVariantList() << m_pos.x() << m_pos.y());
    setAimPos(QVariantList() << m_overlay->aimPosition().x() << m_overlay->aimPosition().y());
}

void CameraWidget::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);

    if (m_camera && m_camera->isActive())
        stopCamera();
}

void CameraWidget::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);

    if (m_camera)
        startCamera();

    m_scrollArea->horizontalScrollBar()->setValue(m_pos.x());
    m_scrollArea->verticalScrollBar()->setValue(m_pos.y());
}

void CameraWidget::initCamera(const QCameraDevice &cameraDevice)
{
    if (m_camera)
        deleteCamera();

    m_cameraDevice = cameraDevice;

    m_camera = new QCamera(cameraDevice, this);
    m_captureSession = new QMediaCaptureSession(this);
    m_captureSession->setCamera(m_camera);
    m_captureSession->setVideoSink(m_videoSurface->sink());

    for (const QCameraFormat &format : cameraDevice.videoFormats())
    {
        if (format.resolution() == m_resolution)
        {
            m_camera->setCameraFormat(format);
            break;
        }
    }

    if (isVisible())
        m_camera->start();
}

void CameraWidget::deleteCamera()
{
    if (!m_camera)
        return;

    m_camera->stop();
    delete m_captureSession;
    m_captureSession = nullptr;
    delete m_camera;
    m_camera = nullptr;

    // Wait last frame
    QCoreApplication::processEvents();

    clearCurrentFrame();
    drawCurrentFrame();
}

void CameraWidget::updateSize()
{
    QSize size = contentsRect().size();

    double aspect = (double)(size.width()) / (size.height());
    double cameraAspect = (double)m_resolution.width() / m_resolution.height();

    if (aspect >= cameraAspect)
    {
        m_videoLabel->resize(size.width() * m_zoom, size.width() / cameraAspect * m_zoom);
    }
    else
    {
        m_videoLabel->resize(size.height() * cameraAspect * m_zoom, size.height() * m_zoom);
    }

    m_overlay->resize(m_videoLabel->size());

    drawCurrentFrame();
}

void CameraWidget::processFrame(const QImage &frame)
{
    Qt::Orientations orientations;

    if (m_mirrorHorizontal) orientations |= Qt::Horizontal;
    if (m_mirrorVertical) orientations |= Qt::Vertical;

    m_currentFrame = frame.flipped(orientations);

    drawCurrentFrame();
}

void CameraWidget::drawCurrentFrame()
{
    if (!m_currentFrame.isNull())
    {
        auto pixmap = QPixmap::fromImage(m_currentFrame)
                          .scaled(m_videoLabel->size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);

        m_videoLabel->setPixmap(pixmap);
    }
}

void CameraWidget::clearCurrentFrame()
{
    m_currentFrame = QImage(1, 1, QImage::Format_RGB32);
    m_currentFrame.fill(Qt::black);
}

void CameraWidget::startCamera()
{
    startStop(true);
}

void CameraWidget::stopCamera()
{
    startStop(false);
}

void CameraWidget::startStop(bool start)
{
    static QTimer *stopTimer = nullptr;
    static QCamera *stopCamera;

    if (!stopTimer)
    {
        stopTimer = new QTimer(this);
        stopTimer->setSingleShot(true);
        stopTimer->setInterval(1000);

        connect(stopTimer, &QTimer::timeout, [=]
                { stopCamera->stop(); });
    }

    if (!m_camera)
        return;

    if (start)
    {
        stopTimer->stop();
        m_camera->start();
    }
    else
    {
        stopCamera = m_camera;
        stopTimer->start();
    }
}