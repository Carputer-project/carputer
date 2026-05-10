#pragma once
#include <QObject>
#include <QProcess>
#include <QVariantList>
#include <QTimer>
#include <QFile>

class InstallManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(QVariantList disks READ disks NOTIFY disksChanged)

public:
    explicit InstallManager(QObject *parent = nullptr);
    ~InstallManager() override;

    bool busy() const { return m_busy; }
    int progress() const { return m_progress; }
    QString statusText() const { return m_statusText; }
    QVariantList disks() const { return m_disks; }

    Q_INVOKABLE void scanDisks();
    Q_INVOKABLE void installToDisk(const QString &device);
    Q_INVOKABLE void cancelInstall();
    Q_INVOKABLE void rebootNow();

signals:
    void busyChanged();
    void progressChanged();
    void statusTextChanged();
    void disksChanged();
    void installComplete(bool success, const QString &message);

private:
    enum Step {
        StepIdle,
        StepPartition,
        StepFormat,
        StepMount,
        StepCopyBin, StepCopyBoot, StepCopyEtc, StepCopyHome,
        StepCopyLib, StepCopyLib64, StepCopyOpt, StepCopyRoot,
        StepCopySbin, StepCopySrv, StepCopyUsr, StepCopyVar,
        StepCopyMkdir,
        StepGrubInstall,
        StepUnmount,
        StepDone
    };

    void advanceTo(Step next);
    void startStep(Step step);
    QString partitionDevice(const QString &disk) const;
    void setProgress(int pct, const QString &status);
    void setError(const QString &message);
    void cleanup();

    bool m_busy = false;
    bool m_cancelRequested = false;
    int m_progress = 0;
    QString m_statusText;
    QVariantList m_disks;
    QString m_targetDevice;
    QString m_targetPartition;
    QString mMountPoint;
    Step m_currentStep = StepIdle;
    QProcess *m_process = nullptr;
    QByteArray m_outputBuffer;
};
