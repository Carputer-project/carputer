#include "installmanager.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QStorageInfo>
#include <QVariantMap>


InstallManager::InstallManager(QObject *parent)
    : QObject(parent)
{
}

InstallManager::~InstallManager()
{
    cleanup();
}

void InstallManager::scanDisks()
{
    m_disks.clear();

    QString bootDevice;
    QStorageInfo rootFs("/");
    QString rootDev = rootFs.device();
    if (rootDev.startsWith("/dev/")) {
        bootDevice = rootDev;
        while (!bootDevice.isEmpty() && bootDevice.right(1).at(0).isDigit())
            bootDevice.chop(1);
        if (bootDevice.endsWith('p'))
            bootDevice.chop(1);
    }

    QDir sysBlock("/sys/block");
    for (const QString &entry : sysBlock.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        if (entry.startsWith("loop") || entry.startsWith("ram") ||
            entry.startsWith("dm-") || entry.startsWith("zram") ||
            entry.startsWith("md") || entry.startsWith("sr"))
            continue;

        QString devPath = "/dev/" + entry;
        if (!QFileInfo::exists(devPath))
            continue;

        if (devPath == bootDevice)
            continue;

        QFile sizeFile("/sys/block/" + entry + "/size");
        if (!sizeFile.open(QIODevice::ReadOnly))
            continue;
        qint64 sectors = sizeFile.readAll().trimmed().toLongLong();
        sizeFile.close();
        if (sectors == 0)
            continue;

        qint64 bytes = sectors * 512LL;

        QString model;
        QFile mf("/sys/block/" + entry + "/device/model");
        if (mf.open(QIODevice::ReadOnly)) {
            model = QString::fromUtf8(mf.readAll()).trimmed();
            mf.close();
        }
        QString vendor;
        QFile vf("/sys/block/" + entry + "/device/vendor");
        if (vf.open(QIODevice::ReadOnly)) {
            vendor = QString::fromUtf8(vf.readAll()).trimmed();
            vf.close();
        }

        QString info = model;
        if (!vendor.isEmpty()) {
            if (!info.isEmpty())
                info = vendor + " " + info;
            else
                info = vendor;
        }
        if (info.isEmpty())
            info = "(generic)";

        QString sizeStr;
        if (bytes >= 1000000000000LL)
            sizeStr = QString::number(bytes / 1000000000000LL) + "." +
                      QString::number((bytes % 1000000000000LL) / 100000000000LL) + " TB";
        else if (bytes >= 1000000000LL)
            sizeStr = QString::number(bytes / 1000000000LL) + " GB";
        else
            sizeStr = QString::number(bytes / 1000000LL) + " MB";

        QVariantMap disk;
        disk["device"] = devPath;
        disk["size"] = sizeStr;
        disk["sizeBytes"] = static_cast<qint64>(bytes);
        disk["model"] = info;
        m_disks.append(disk);
    }

    emit disksChanged();
}

void InstallManager::installToDisk(const QString &device)
{
    if (m_busy)
        return;

    m_targetDevice = device;
    m_targetPartition = partitionDevice(device);
    mMountPoint = QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                  + "/carputer-install";

    m_busy = true;
    m_cancelRequested = false;
    m_progress = 0;
    m_currentStep = StepIdle;
    setProgress(0, "Starting installation...");
    emit busyChanged();

    advanceTo(StepPartition);
}

void InstallManager::cancelInstall()
{
    m_cancelRequested = true;
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(3000);
    }
    cleanup();
    m_busy = false;
    emit busyChanged();
    setProgress(0, "Cancelled");
    emit installComplete(false, "Installation cancelled by user");
}

void InstallManager::rebootNow()
{
    QProcess::startDetached("/bin/systemctl", {"reboot"});
}

void InstallManager::advanceTo(Step next)
{
    m_currentStep = next;
    startStep(next);
}

void InstallManager::startStep(Step step)
{
    if (m_cancelRequested)
        return;

    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }

    m_outputBuffer.clear();
    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        QByteArray data = m_process->readAllStandardOutput();
        m_outputBuffer += data;
        QString line = QString::fromUtf8(data).trimmed();
        if (!line.isEmpty())
            qDebug() << "[InstallManager]" << line;
    });

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int code, QProcess::ExitStatus status) {
        if (m_cancelRequested)
            return;

        if (code != 0 || status != QProcess::NormalExit) {
            QString err = QString::fromUtf8(m_outputBuffer).trimmed();
            setError(QString("Step %1 failed (exit %2): %3")
                .arg(static_cast<int>(m_currentStep)).arg(code).arg(err));
            return;
        }

        switch (m_currentStep) {
        case StepPartition:
            QTimer::singleShot(1000, this, [this]() { advanceTo(StepFormat); });
            break;
        case StepFormat:
            advanceTo(StepMount);
            break;
        case StepMount:      advanceTo(StepCopyBin);  break;
        case StepCopyBin:    advanceTo(StepCopyBoot); break;
        case StepCopyBoot:   advanceTo(StepCopyEtc);  break;
        case StepCopyEtc:    advanceTo(StepCopyHome); break;
        case StepCopyHome:   advanceTo(StepCopyLib);  break;
        case StepCopyLib:    advanceTo(StepCopyLib64); break;
        case StepCopyLib64:  advanceTo(StepCopyOpt);  break;
        case StepCopyOpt:    advanceTo(StepCopyRoot); break;
        case StepCopyRoot:   advanceTo(StepCopySbin); break;
        case StepCopySbin:   advanceTo(StepCopySrv);  break;
        case StepCopySrv:    advanceTo(StepCopyUsr);  break;
        case StepCopyUsr:    advanceTo(StepCopyVar);  break;
        case StepCopyVar:    advanceTo(StepCopyMkdir); break;
        case StepCopyMkdir:  advanceTo(StepGrubInstall); break;
        case StepGrubInstall: advanceTo(StepUnmount); break;
        case StepUnmount:
            setProgress(100, "Installation complete!");
            QDir().rmdir(mMountPoint);
            m_busy = false;
            m_currentStep = StepDone;
            emit busyChanged();
            emit installComplete(true, "Installation complete! You can now reboot.");
            cleanup();
            break;
        default:
            break;
        }
    });

    connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        QString out = QString::fromUtf8(m_process->readAllStandardOutput()).trimmed();
        if (!out.isEmpty())
            qDebug() << "[InstallManager]" << out;
    });

    connect(m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError) {
        if (!m_cancelRequested)
            setError("Process error: " + m_process->errorString());
    });

    switch (step) {
    case StepPartition: {
        setProgress(5, "Partitioning disk...");
        m_process->setProgram("fdisk");
        m_process->setArguments({m_targetDevice});
        connect(m_process, &QProcess::started, this, [this]() {
            m_process->write("o\nn\np\n1\n2048\n\na\n1\nw\n");
            m_process->closeWriteChannel();
        });
        m_process->start();
        break;
    }
    case StepFormat:
        setProgress(15, "Formatting ext4 filesystem...");
        m_process->setProgram("mkfs.ext4");
        m_process->setArguments({"-F", "-L", "rootfs", m_targetPartition});
        m_process->start();
        break;
    case StepMount:
        setProgress(25, "Mounting partition...");
        QDir().mkpath(mMountPoint);
        m_process->setProgram("mount");
        m_process->setArguments({m_targetPartition, mMountPoint});
        m_process->start();
        break;
    case StepCopyBin:
        setProgress(32, "Copying /bin...");
        m_process->setProgram("cp"); m_process->setArguments({"-a", "/bin", mMountPoint});
        m_process->start(); break;
    case StepCopyBoot:
        setProgress(36, "Copying /boot...");
        m_process->setProgram("cp"); m_process->setArguments({"-a", "/boot", mMountPoint});
        m_process->start(); break;
    case StepCopyEtc:
        setProgress(40, "Copying /etc...");
        m_process->setProgram("cp"); m_process->setArguments({"-a", "/etc", mMountPoint});
        m_process->start(); break;
    case StepCopyHome:
        setProgress(43, "Copying /home...");
        m_process->setProgram("cp"); m_process->setArguments({"-a", "/home", mMountPoint});
        m_process->start(); break;
    case StepCopyLib:
        setProgress(47, "Copying /lib...");
        m_process->setProgram("cp"); m_process->setArguments({"-a", "/lib", mMountPoint});
        m_process->start(); break;
    case StepCopyLib64:
        setProgress(50, "Copying /lib64...");
        m_process->setProgram("cp"); m_process->setArguments({"-a", "/lib64", mMountPoint});
        m_process->start(); break;
    case StepCopyOpt:
        setProgress(53, "Copying /opt...");
        m_process->setProgram("cp"); m_process->setArguments({"-a", "/opt", mMountPoint});
        m_process->start(); break;
    case StepCopyRoot:
        setProgress(56, "Copying /root...");
        m_process->setProgram("cp"); m_process->setArguments({"-a", "/root", mMountPoint});
        m_process->start(); break;
    case StepCopySbin:
        setProgress(60, "Copying /sbin...");
        m_process->setProgram("cp"); m_process->setArguments({"-a", "/sbin", mMountPoint});
        m_process->start(); break;
    case StepCopySrv:
        setProgress(63, "Copying /srv...");
        m_process->setProgram("cp"); m_process->setArguments({"-a", "/srv", mMountPoint});
        m_process->start(); break;
    case StepCopyUsr:
        setProgress(67, "Copying /usr...");
        m_process->setProgram("cp"); m_process->setArguments({"-a", "/usr", mMountPoint});
        m_process->start(); break;
    case StepCopyVar:
        setProgress(70, "Copying /var...");
        m_process->setProgram("cp"); m_process->setArguments({"-a", "/var", mMountPoint});
        m_process->start(); break;
    case StepCopyMkdir:
        setProgress(73, "Creating mount points...");
        m_process->setProgram("mkdir");
        m_process->setArguments({"-p",
            mMountPoint + "/dev", mMountPoint + "/proc", mMountPoint + "/sys",
            mMountPoint + "/tmp", mMountPoint + "/run", mMountPoint + "/mnt",
            mMountPoint + "/media"});
        m_process->start();
        break;
    case StepGrubInstall:
        setProgress(78, "Installing bootloader...");
        m_process->setProgram("grub-install");
        m_process->setArguments({
            "--target=i386-pc",
            "--boot-directory=" + mMountPoint + "/boot",
            m_targetDevice
        });
        m_process->start();
        break;
    case StepUnmount:
        setProgress(95, "Finalizing...");
        m_process->setProgram("sh");
        m_process->setArguments({"-c",
            "sync && umount " + mMountPoint + " 2>/dev/null; rm -rf " + mMountPoint});
        m_process->start();
        break;
    default:
        break;
    }
}

void InstallManager::setProgress(int pct, const QString &status)
{
    if (m_progress != pct && !m_cancelRequested) {
        m_progress = pct;
        emit progressChanged();
    }
    if (m_statusText != status && !m_cancelRequested) {
        m_statusText = status;
        emit statusTextChanged();
    }
}

void InstallManager::setError(const QString &message)
{
    qWarning() << "[InstallManager] ERROR:" << message;
    m_statusText = "Error: " + message;
    m_progress = 0;
    m_busy = false;
    m_currentStep = StepIdle;
    emit statusTextChanged();
    emit progressChanged();
    emit busyChanged();
    emit installComplete(false, message);
    cleanup();
}

void InstallManager::cleanup()
{
    if (m_process) {
        if (m_process->state() != QProcess::NotRunning) {
            m_process->kill();
            m_process->waitForFinished(2000);
        }
        m_process->deleteLater();
        m_process = nullptr;
    }
    if (!mMountPoint.isEmpty()) {
        QProcess um;
        um.start("umount", {mMountPoint});
        um.waitForFinished(3000);
        QDir().rmdir(mMountPoint);
    }
}

QString InstallManager::partitionDevice(const QString &disk) const
{
    if (disk.contains("nvme") || disk.contains("mmcblk") || disk.contains("loop"))
        return disk + "p1";
    return disk + "1";
}


