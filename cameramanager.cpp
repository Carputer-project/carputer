#include "cameramanager.h"
#include "videoframeprovider.h"
#include <QDebug>
#include <QDir>
#include <gst/video/video.h>

GstFlowReturn onCameraSample(GstAppSink *appsink, gpointer data)
{
    auto *self = static_cast<CameraManager*>(data);
    if (!self || !self->m_videoProvider) return GST_FLOW_OK;

    GstSample *sample = gst_app_sink_pull_sample(appsink);
    if (!sample) return GST_FLOW_OK;

    GstCaps *caps = gst_sample_get_caps(sample);
    GstBuffer *buffer = gst_sample_get_buffer(sample);
    if (!caps || !buffer) { gst_sample_unref(sample); return GST_FLOW_OK; }

    GstVideoInfo vinfo;
    if (!gst_video_info_from_caps(&vinfo, caps)) { gst_sample_unref(sample); return GST_FLOW_OK; }

    GstMapInfo info;
    gst_buffer_map(buffer, &info, GST_MAP_READ);

    int w = GST_VIDEO_INFO_WIDTH(&vinfo);
    int h = GST_VIDEO_INFO_HEIGHT(&vinfo);
    int stride = GST_VIDEO_INFO_PLANE_STRIDE(&vinfo, 0);

    QImage img;
    GstVideoFormat fmt = GST_VIDEO_INFO_FORMAT(&vinfo);
    if (fmt == GST_VIDEO_FORMAT_RGB) {
        img = QImage(info.data, w, h, stride, QImage::Format_RGB888).copy();
    } else if (fmt == GST_VIDEO_FORMAT_BGRx || fmt == GST_VIDEO_FORMAT_BGRA) {
        img = QImage(info.data, w, h, stride, QImage::Format_ARGB32).rgbSwapped().copy();
    } else {
        img = QImage(w, h, QImage::Format_RGB888);
        img.fill(Qt::black);
    }

    gst_buffer_unmap(buffer, &info);
    gst_sample_unref(sample);

    if (!img.isNull())
        self->m_videoProvider->updateFrame(QStringLiteral("camera"), img);
    return GST_FLOW_OK;
}

CameraManager::CameraManager(QObject *parent) : QObject(parent)
{
    gst_init(nullptr, nullptr);
    scanDevices();
}

CameraManager::~CameraManager()
{
    stopStream();
}

void CameraManager::setVideoFrameProvider(VideoFrameProvider *provider)
{
    m_videoProvider = provider;
}

void CameraManager::scanDevices()
{
    m_availableDevices.clear();
    QDir devDir("/dev");
    QStringList videoDevices = devDir.entryList(QStringList() << "video*", QDir::System);
    for (const QString &dev : videoDevices)
        m_availableDevices.append("/dev/" + dev);
    if (m_availableDevices.isEmpty())
        m_availableDevices.append("/dev/video0");
    if (m_device.isEmpty())
        m_device = m_availableDevices.first();
    emit availableDevicesChanged();
}

void CameraManager::setupPipeline()
{
    if (m_pipeline) return;

    m_pipeline = gst_pipeline_new("carputer-camera");
    GstElement *src = gst_element_factory_make("v4l2src", "camera-source");
    GstElement *conv = gst_element_factory_make("videoconvert", "camera-convert");
    GstElement *scale = gst_element_factory_make("videoscale", "camera-scale");
    m_appsink = gst_element_factory_make("appsink", "camera-appsink");

    if (!src || !conv || !scale || !m_appsink) {
        qWarning() << "[CAM] Failed to create pipeline elements";
        if (m_pipeline) { gst_object_unref(m_pipeline); m_pipeline = nullptr; }
        return;
    }

    g_object_set(src, "device", m_device.toUtf8().constData(), nullptr);

    GstCaps *caps = gst_caps_new_simple("video/x-raw",
        "format", G_TYPE_STRING, "RGB",
        "width", G_TYPE_INT, 640,
        "height", G_TYPE_INT, 480,
        nullptr);
    gst_app_sink_set_caps(GST_APP_SINK(m_appsink), caps);
    gst_caps_unref(caps);

    g_object_set(m_appsink, "sync", FALSE, "drop", TRUE, "max-buffers", 1, nullptr);

    GstAppSinkCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.new_sample = onCameraSample;
    gst_app_sink_set_callbacks(GST_APP_SINK(m_appsink), &cb, this, nullptr);

    gst_bin_add_many(GST_BIN(m_pipeline), src, conv, scale, m_appsink, nullptr);
    gst_element_link_many(src, conv, scale, m_appsink, nullptr);
}

void CameraManager::teardownPipeline()
{
    if (m_pipeline) {
        if (m_busWatchId > 0) {
            g_source_remove(m_busWatchId);
            m_busWatchId = 0;
        }
        gst_element_set_state(m_pipeline, GST_STATE_NULL);
        gst_object_unref(m_pipeline);
        m_pipeline = nullptr;
        m_appsink = nullptr;
    }
}

void CameraManager::startStream(const QString &dev)
{
    if (!dev.isEmpty()) {
        m_device = dev;
        emit deviceChanged();
    }
    if (m_streaming) stopStream();

    setupPipeline();
    if (!m_pipeline) {
        emit errorOccurred(QStringLiteral("Failed to create camera pipeline"));
        return;
    }

    gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
    m_streaming = true;
    emit streamingChanged();
    qDebug() << "[CAM] Stream started on" << m_device;
}

void CameraManager::stopStream()
{
    if (m_streaming) {
        m_streaming = false;
        emit streamingChanged();
    }
    teardownPipeline();
    if (m_videoProvider)
        m_videoProvider->updateFrame("camera", QImage());
    qDebug() << "[CAM] Stream stopped";
}

void CameraManager::setDevice(const QString &dev)
{
    if (m_device != dev) {
        m_device = dev;
        emit deviceChanged();
    }
}
