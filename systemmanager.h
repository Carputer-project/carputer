#pragma once
#include <QObject>
#include <QProcess>
#include <QVariantList>

class SystemManager : public QObject
{
    Q_OBJECT
public:
    explicit SystemManager(QObject *parent = nullptr);

    Q_INVOKABLE void shutdown();
    Q_INVOKABLE void reboot();
    Q_INVOKABLE void powerOff();
    Q_INVOKABLE void emergencyStop();
    Q_INVOKABLE void launchFileManager();
    Q_INVOKABLE void launchTerminal();
    Q_INVOKABLE void executeCommand(const QString &command, const QStringList &args = QStringList());

    Q_INVOKABLE void startShell();
    Q_INVOKABLE void stopShell();
    Q_INVOKABLE void sendShellCommand(const QString &cmd);

    Q_INVOKABLE QVariantList listDirectory(const QString &path);
    Q_INVOKABLE bool isDir(const QString &path);
    Q_INVOKABLE void playFile(const QString &path);

    Q_INVOKABLE QString getSystemUptime();
    Q_INVOKABLE QString getSystemLoad();
    Q_INVOKABLE QString getDiskUsage(const QString &path = "/");
    Q_INVOKABLE void restartService(const QString &serviceName);
    Q_INVOKABLE QString getServiceStatus(const QString &serviceName);
    Q_INVOKABLE QString runShellCommand(const QString &cmd);

signals:
    void commandFinished(int exitCode);
    void commandError(const QString &error);
    void shellOutput(const QString &text);

private:
    QProcess *shellProcess = nullptr;
};
