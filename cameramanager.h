#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <gst/gst.h>
#include <gst/app/gstappsink.h>

class VideoFrameProvider;

class CameraManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool streaming READ streaming NOTIFY streamingChanged)
    Q_PROPERTY(QString device READ device WRITE setDevice NOTIFY deviceChanged)
    Q_PROPERTY(QStringList availableDevices READ availableDevices NOTIFY availableDevicesChanged)

public:
    explicit CameraManager(QObject *parent = nullptr);
    ~CameraManager();

    void setVideoFrameProvider(VideoFrameProvider *provider);

    bool streaming() const { return m_streaming; }
    QString device() const { return m_device; }
    QStringList availableDevices() const { return m_availableDevices; }

    Q_INVOKABLE void startStream(const QString &dev = QString());
    Q_INVOKABLE void stopStream();
    Q_INVOKABLE void scanDevices();

public slots:
    void setDevice(const QString &dev);

signals:
    void streamingChanged();
    void deviceChanged();
    void availableDevicesChanged();
    void errorOccurred(const QString &msg);

private:
    void setupPipeline();
    void teardownPipeline();

    friend GstFlowReturn onCameraSample(GstAppSink *appsink, gpointer data);

    VideoFrameProvider *m_videoProvider = nullptr;
    bool m_streaming = false;
    QString m_device;
    QStringList m_availableDevices;

    GstElement *m_pipeline = nullptr;
    GstElement *m_appsink = nullptr;
    guint m_busWatchId = 0;
};
