#pragma once

#include <QObject>
#include <QProcess>
#include <QTimer>

class CameraManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool streaming READ streaming NOTIFY streamingChanged)
    Q_PROPERTY(QString device READ device WRITE setDevice NOTIFY deviceChanged)
    Q_PROPERTY(QStringList availableDevices READ availableDevices NOTIFY availableDevicesChanged)

public:
    explicit CameraManager(QObject *parent = nullptr);
    ~CameraManager();

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

private slots:
    void onProcessStarted();
    void onProcessError(QProcess::ProcessError error);
    void onProcessFinished(int code, QProcess::ExitStatus status);

private:
    QProcess *m_process = nullptr;
    bool m_streaming = false;
    QString m_device;
    QStringList m_availableDevices;
};
