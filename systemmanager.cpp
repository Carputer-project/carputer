#include "systemmanager.h"
#include <QDir>
#include <QFileInfo>
#include <QStorageInfo>
#include <QVariantMap>

SystemManager::SystemManager(QObject *parent) : QObject(parent) {}

void SystemManager::shutdown()
{
    QProcess::startDetached("/bin/systemctl", {"poweroff"});
}

void SystemManager::reboot()
{
    QProcess::startDetached("/bin/systemctl", {"reboot"});
}

void SystemManager::powerOff()
{
    QProcess::startDetached("/bin/systemctl", {"poweroff"});
}

void SystemManager::emergencyStop()
{
    QProcess::startDetached("/bin/systemctl", {"halt"});
}

void SystemManager::launchTerminal()
{
    startShell();
}

void SystemManager::launchFileManager()
{
    // handled by embedded file browser in QML
}


void SystemManager::executeCommand(const QString &command, const QStringList &args)
{
    QProcess *proc = new QProcess(this);
    proc->setProcessChannelMode(QProcess::MergedChannels);
    connect(proc, &QProcess::readyReadStandardOutput, this, [this, proc]() {
        emit shellOutput(QString::fromUtf8(proc->readAllStandardOutput()));
    });
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc](int code, QProcess::ExitStatus) {
        // Flush any remaining output before signalling completion
        QString remaining = QString::fromUtf8(proc->readAllStandardOutput());
        if (!remaining.isEmpty())
            emit shellOutput(remaining);
        emit commandFinished(code);
        proc->deleteLater();
    });
    connect(proc, &QProcess::errorOccurred, this, [this, proc](QProcess::ProcessError) {
        emit commandError(proc->errorString());
        proc->deleteLater();
    });
    proc->start(command, args);
}

void SystemManager::startShell()
{
    if (shellProcess) return;
    shellProcess = new QProcess(this);
    shellProcess->setProcessChannelMode(QProcess::MergedChannels);
    connect(shellProcess, &QProcess::readyReadStandardOutput, this, [this]() {
        emit shellOutput(QString::fromUtf8(shellProcess->readAllStandardOutput()));
    });
    shellProcess->start("/bin/sh", {"-i"});
}

void SystemManager::stopShell()
{
    if (shellProcess) {
        shellProcess->kill();
        shellProcess->deleteLater();
        shellProcess = nullptr;
    }
}

void SystemManager::sendShellCommand(const QString &cmd)
{
    if (shellProcess && shellProcess->state() == QProcess::Running)
        shellProcess->write((cmd + "\n").toUtf8());
}

QVariantList SystemManager::listDirectory(const QString &path)
{
    QVariantList result;
    QDir dir(path);
    if (!dir.exists()) return result;

    if (path != "/") {
        QVariantMap parent;
        parent["name"]  = "..";
        parent["isDir"] = true;
        parent["size"]  = "";
        result.append(parent);
    }

    dir.setFilter(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden);
    dir.setSorting(QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);

    for (const QFileInfo &info : dir.entryInfoList()) {
        QVariantMap entry;
        entry["name"]  = info.fileName();
        entry["isDir"] = info.isDir();
        if (info.isDir()) {
            entry["size"] = "";
        } else {
            qint64 sz = info.size();
            if (sz < 1024)
                entry["size"] = QString("%1B").arg(sz);
            else if (sz < 1024 * 1024)
                entry["size"] = QString("%1K").arg(sz / 1024);
            else
                entry["size"] = QString("%1M").arg(sz / (1024 * 1024));
        }
        result.append(entry);
    }
    return result;
}

bool SystemManager::isDir(const QString &path)
{
    return QFileInfo(path).isDir();
}

void SystemManager::playFile(const QString &path)
{
    emit shellOutput(QString("[PLAY] %1\n").arg(path));
}

QString SystemManager::getSystemUptime()
{
    QProcess proc;
    proc.start("/bin/sh", {"-c", "uptime | awk -F'up' '{print $2}' | cut -d',' -f1"});
    proc.waitForFinished(2000);
    return QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
}

QString SystemManager::getSystemLoad()
{
    QProcess proc;
    proc.start("/bin/sh", {"-c", "cat /proc/loadavg | awk '{print $1, $2, $3}'"});
    proc.waitForFinished(2000);
    return QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
}

QString SystemManager::getDiskUsage(const QString &path)
{
    QStorageInfo storage(path);
    if (!storage.isValid())
        return QStringLiteral("N/A");
    constexpr qint64 MB = 1024LL * 1024;
    constexpr qint64 GB = 1024LL * 1024 * 1024;
    auto fmt = [MB, GB](qint64 bytes) -> QString {
        if (bytes < GB)
            return QString("%1M").arg(bytes / MB);
        return QString("%1G").arg(bytes / GB);
    };
    return QString("%1 used: %2 avail: %3")
        .arg(fmt(storage.bytesTotal()),
             fmt(storage.bytesTotal() - storage.bytesAvailable()),
             fmt(storage.bytesAvailable()));
}

void SystemManager::restartService(const QString &serviceName)
{
    QProcess::startDetached("/bin/systemctl", {"restart", serviceName});
    emit shellOutput(QString("[SERVICE] Restarting %1...\n").arg(serviceName));
}

QString SystemManager::getServiceStatus(const QString &serviceName)
{
    QProcess proc;
    proc.start("/bin/systemctl", {"is-active", serviceName});
    proc.waitForFinished(2000);
    return QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
}

QString SystemManager::runShellCommand(const QString &cmd)
{
    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    QString safeCmd = cmd;
    safeCmd.replace(";", " ").replace("&&", " ").replace("||", " ");
    safeCmd.replace("|", " ").replace(">", " ").replace("<", " ");
    safeCmd.replace("`", " ").replace("$", " ").replace("(", " ").replace(")", " ");
    proc.start("/bin/sh", {"-c", safeCmd});
    proc.waitForFinished(3000);
    return QString::fromUtf8(proc.readAllStandardOutput()).trimmed();
}
