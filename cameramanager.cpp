#include "cameramanager.h"
#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QStringList>

CameraManager::CameraManager(QObject *parent) : QObject(parent)
{
    m_process = new QProcess(this);
    connect(m_process, &QProcess::started, this, &CameraManager::onProcessStarted);
    connect(m_process, QOverload<QProcess::ProcessError>::of(&QProcess::error),
            this, &CameraManager::onProcessError);
    connect(m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &CameraManager::onProcessFinished);

    scanDevices();
}

CameraManager::~CameraManager()
{
    stopStream();
}

void CameraManager::scanDevices()
{
    m_availableDevices.clear();
    QDir devDir("/dev");
    QStringList videoDevices = devDir.entryList(QStringList() << "video*", QDir::System);
    for (const QString &dev : videoDevices) {
        m_availableDevices.append("/dev/" + dev);
    }
    if (m_availableDevices.isEmpty()) {
        m_availableDevices.append("/dev/video0");
    }
    if (m_device.isEmpty()) {
        m_device = m_availableDevices.first();
    }
    emit availableDevicesChanged();
}

void CameraManager::startStream(const QString &dev)
{
    if (!dev.isEmpty()) {
        m_device = dev;
        emit deviceChanged();
    }

    if (m_streaming) {
        stopStream();
    }

    qDebug() << "Starting camera stream on:" << m_device;

    QStringList args;
    args << "v4l2src" << QString("device=%1").arg(m_device)
         << "!" << "videoconvert" << "!" << "waylandsink";

    m_process->start("gst-launch-1.0", args);
}

void CameraManager::stopStream()
{
    if (m_process->state() != QProcess::NotRunning) {
        m_process->terminate();
        m_process->waitForFinished(1000);
    }
    m_streaming = false;
    emit streamingChanged();
}

void CameraManager::setDevice(const QString &dev)
{
    if (m_device != dev) {
        m_device = dev;
        emit deviceChanged();
        if (m_streaming) {
            startStream();
        }
    }
}

void CameraManager::onProcessStarted()
{
    m_streaming = true;
    emit streamingChanged();
}

void CameraManager::onProcessError(QProcess::ProcessError error)
{
    qWarning() << "Camera process error:" << error << m_process->errorString();
    emit errorOccurred(m_process->errorString());
    m_streaming = false;
    emit streamingChanged();
}

void CameraManager::onProcessFinished(int code, QProcess::ExitStatus status)
{
    qDebug() << "Camera process finished:" << code << status;
    m_streaming = false;
    emit streamingChanged();
}
