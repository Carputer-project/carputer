#include "internalwifimanager.h"
#include <QDebug>
#include <QRegularExpression>
#include <QThread>

InternalWiFiManager::InternalWiFiManager(QObject *parent)
    : QObject(parent)
{
    m_statusTimer = new QTimer(this);
    m_statusTimer->setInterval(5000);
    connect(m_statusTimer, &QTimer::timeout, this, &InternalWiFiManager::checkConnectionStatus);
    m_statusTimer->start();
    checkConnectionStatus();
    
    // Auto-connect to Carputer_ECU after 3 seconds if not already connected
    QTimer::singleShot(3000, this, [this]() {
        if (!m_connected) {
            connectToCarputerECU();
        }
    });
}

InternalWiFiManager::~InternalWiFiManager()
{
    if (m_scanProcess.state() != QProcess::NotRunning)
        m_scanProcess.kill();
    if (m_connectProcess.state() != QProcess::NotRunning)
        m_connectProcess.kill();
}

void InternalWiFiManager::scanNetworks()
{
    if (m_scanProcess.state() != QProcess::NotRunning) {
        setStatusText("Scan already in progress...");
        return;
    }

    setStatusText("Scanning for networks...");
    m_networks.clear();
    emit networksChanged();

    m_scanProcess.setProgram("iw");
    m_scanProcess.setArguments({"dev", m_interface, "scan"});
    connect(&m_scanProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &InternalWiFiManager::onScanFinished, Qt::UniqueConnection);
    m_scanProcess.start();
}

void InternalWiFiManager::connectToNetwork(const QString &ssid, const QString &password)
{
    if (ssid.isEmpty()) {
        setStatusText("Error: No SSID provided");
        return;
    }

    setStatusText(QString("Connecting to %1...").arg(ssid));
    disconnectNetwork();

    // Use wpa_cli to connect
    QProcess *proc = new QProcess(this);
    QStringList commands;

    if (password.isEmpty()) {
        // Open network
        commands << QString("wpa_cli -i %1 add_network").arg(m_interface);
        commands << QString("wpa_cli -i %1 set_network 0 ssid '%2'").arg(m_interface, ssid);
        commands << QString("wpa_cli -i %1 set_network 0 key_mgmt NONE").arg(m_interface);
        commands << QString("wpa_cli -i %1 enable_network 0").arg(m_interface);
    } else {
        commands << QString("wpa_cli -i %1 add_network").arg(m_interface);
        commands << QString("wpa_cli -i %1 set_network 0 ssid '%2'").arg(m_interface, ssid);
        commands << QString("wpa_cli -i %1 set_network 0 psk '%2'").arg(m_interface, password);
        commands << QString("wpa_cli -i %1 enable_network 0").arg(m_interface);
    }

    // Execute commands sequentially
    QString fullCmd = commands.join(" && ");
    proc->setProgram("sh");
    proc->setArguments({"-c", fullCmd});
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &InternalWiFiManager::onConnectFinished);
    proc->start();
}

void InternalWiFiManager::disconnectNetwork()
{
    setStatusText("Disconnecting...");
    QProcess *proc = new QProcess(this);
    proc->setProgram("wpa_cli");
    proc->setArguments({"-i", m_interface, "disable_network", "0"});
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &InternalWiFiManager::onDisconnectFinished);
    proc->start();
}

void InternalWiFiManager::forgetNetwork(const QString &ssid)
{
    Q_UNUSED(ssid);
    QProcess *proc = new QProcess(this);
    proc->setProgram("wpa_cli");
    proc->setArguments({"-i", m_interface, "remove_network", "0"});
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc](int, QProcess::ExitStatus) {
        setStatusText("Network forgotten");
        proc->deleteLater();
    });
    proc->start();
}

void InternalWiFiManager::onScanFinished()
{
    static const QRegularExpression ssidRx("SSID: (.+)");
    static const QRegularExpression signalRx("signal: (-?\\d+) dBm");

    QString output = m_scanProcess.readAllStandardOutput();
    QStringList lines = output.split("\n");
    QStringList networks;

    for (const QString &line : lines) {
        QRegularExpressionMatch ssidMatch = ssidRx.match(line);
        if (ssidMatch.hasMatch()) {
            QString ssid = ssidMatch.captured(1).trimmed();
            if (!ssid.isEmpty() && !networks.contains(ssid)) {
                networks.append(ssid);
            }
        }
    }

    updateNetworks(networks);
    setStatusText(QString("Found %1 networks").arg(networks.size()));
}

void InternalWiFiManager::onConnectFinished()
{
    QProcess *proc = qobject_cast<QProcess*>(sender());
    if (!proc) return;

    if (proc->exitCode() == 0) {
        setStatusText("Connection initiated, waiting for IP...");
        QTimer::singleShot(3000, this, &InternalWiFiManager::checkConnectionStatus);
    } else {
        QString error = proc->readAllStandardError();
        setStatusText("Connection failed");
        emit connectionFailed(error);
    }
    proc->deleteLater();
}

void InternalWiFiManager::onDisconnectFinished()
{
    QProcess *proc = qobject_cast<QProcess*>(sender());
    if (!proc) return;
    setConnected(false);
    setSsid("");
    setIpAddress("");
    setSignalStrength(0);
    setStatusText("Disconnected");
    proc->deleteLater();
}

void InternalWiFiManager::checkConnectionStatus()
{
    // Check link status
    QProcess proc;
    proc.setProgram("iw");
    proc.setArguments({"dev", m_interface, "link"});
    proc.start();
    proc.waitForFinished(3000);

    QString output = proc.readAllStandardOutput();
    bool connected = output.contains("Connected to");
    setConnected(connected);

    if (connected) {
        static const QRegularExpression ssidRx("SSID: (.+)");
        static const QRegularExpression signalRx("signal: (-?\\d+) dBm");
        static const QRegularExpression ipRx("inet (\\d+\\.\\d+\\.\\d+\\.\\d+)/");

        // Extract SSID
        QRegularExpressionMatch match = ssidRx.match(output);
        if (match.hasMatch())
            setSsid(match.captured(1).trimmed());

        // Extract signal strength
        QRegularExpressionMatch signalMatch = signalRx.match(output);
        if (signalMatch.hasMatch())
            setSignalStrength(signalMatch.captured(1).toInt());

        // Get IP address
        QProcess ipProc;
        ipProc.setProgram("ip");
        ipProc.setArguments({"addr", "show", m_interface});
        ipProc.start();
        ipProc.waitForFinished(3000);
        QString ipOutput = ipProc.readAllStandardOutput();
        QRegularExpressionMatch ipMatch = ipRx.match(ipOutput);
        if (ipMatch.hasMatch())
            setIpAddress(ipMatch.captured(1));

        setStatusText(QString("Connected to %1 (%2 dBm)").arg(m_ssid).arg(m_signalStrength));
    } else {
        setSsid("");
        setIpAddress("");
        setSignalStrength(0);
        setStatusText("Not connected");
    }
}

void InternalWiFiManager::setConnected(bool c)
{
    if (m_connected != c) {
        m_connected = c;
        emit connectedChanged();
    }
}

void InternalWiFiManager::setSsid(const QString &s)
{
    if (m_ssid != s) {
        m_ssid = s;
        emit ssidChanged();
    }
}

void InternalWiFiManager::setIpAddress(const QString &ip)
{
    if (m_ipAddress != ip) {
        m_ipAddress = ip;
        emit ipAddressChanged();
    }
}

void InternalWiFiManager::setSignalStrength(int strength)
{
    if (m_signalStrength != strength) {
        m_signalStrength = strength;
        emit signalStrengthChanged();
    }
}

void InternalWiFiManager::setStatusText(const QString &text)
{
    if (m_statusText != text) {
        m_statusText = text;
        emit statusTextChanged();
    }
}

void InternalWiFiManager::updateNetworks(const QStringList &networks)
{
    if (m_networks != networks) {
        m_networks = networks;
        emit networksChanged();
    }
}

void InternalWiFiManager::setStaticIP(const QString &ip, const QString &netmask, const QString &gateway)
{
    setStatusText(QString("Setting static IP %1...").arg(ip));
    
        // Remove existing IP addresses
        QProcess *proc = new QProcess(this);
        QStringList commands;
        commands << QString("ip addr flush dev %1").arg(m_interface);
        int cidr = (netmask == "255.255.255.0" ? 24 : netmask == "255.255.0.0" ? 16 : netmask == "255.0.0.0" ? 8 : 24);
        commands << QString("ip addr add %1/%2 dev %3").arg(ip).arg(cidr).arg(m_interface);

        if (!gateway.isEmpty()) {
            commands << QString("ip route add default via %1 dev %2").arg(gateway).arg(m_interface);
        }
    
    QString fullCmd = commands.join(" && ");
    proc->setProgram("sh");
    proc->setArguments({"-c", fullCmd});
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc, ip]() {
        if (proc->exitCode() == 0) {
            setIpAddress(ip);
            setStatusText(QString("Static IP set: %1").arg(ip));
            } else {
                setStatusText("wpa_supplicant started, waiting for connection...");
                // Wait up to 10 seconds for connection, checking every second
                m_connectRetryCount = 0;
                QTimer *waitTimer = new QTimer(this);
                connect(waitTimer, &QTimer::timeout, this, [this, waitTimer]() {
                    checkConnectionStatus();
                    if (m_connected && m_ssid == "Carputer_ECU") {
                        setStatusText("Connected to Carputer_ECU, setting static IP...");
                        setStaticIP("192.168.4.3", "255.255.255.0", "192.168.4.1");
                        waitTimer->stop();
                        waitTimer->deleteLater();
                    } else if (++m_connectRetryCount > 10) {
                        setStatusText("Timeout waiting for Carputer_ECU connection");
                        waitTimer->stop();
                        waitTimer->deleteLater();
                    }
                });
                waitTimer->start(1000);
            }
        proc->deleteLater();
    });
    proc->start();
}

void InternalWiFiManager::connectToCarputerECU()
{
    setStatusText("Connecting to Carputer_ECU with static IP...");
    
    // Kill any existing wpa_supplicant
    QProcess::execute("killall", {"wpa_supplicant"});
    QThread::sleep(1);
    
    // Start wpa_supplicant with config file
    QProcess *proc = new QProcess(this);
    proc->setProgram("wpa_supplicant");
    proc->setArguments({"-B", "-i", m_interface, "-c", "/etc/wpa_supplicant.conf"});
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, proc](int exitCode, QProcess::ExitStatus) {
        if (exitCode == 0) {
            setStatusText("wpa_supplicant started, waiting for connection...");
            // Wait up to 10 seconds for connection, checking every second
            m_connectRetryCount = 0;
            QTimer *waitTimer = new QTimer(this);
            connect(waitTimer, &QTimer::timeout, this, [this, waitTimer]() {
                checkConnectionStatus();
                if (m_connected && m_ssid == "Carputer_ECU") {
                    setStatusText("Connected to Carputer_ECU, setting static IP...");
                    setStaticIP("192.168.4.3", "255.255.255.0", "192.168.4.1");
                    waitTimer->stop();
                    waitTimer->deleteLater();
                } else if (++m_connectRetryCount > 10) {
                    setStatusText("Timeout waiting for Carputer_ECU connection");
                    waitTimer->stop();
                    waitTimer->deleteLater();
                }
            });
            waitTimer->start(1000);
        } else {
            QString error = proc->readAllStandardError();
            setStatusText(QString("wpa_supplicant failed: %1").arg(error));
        }
        proc->deleteLater();
    });
    proc->start();
}
