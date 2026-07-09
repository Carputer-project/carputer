#include "dtcmanager.h"
#include <QDebug>
#include <QNetworkDatagram>
#include <QFile>
#include <QDir>
#include <QJsonValue>
#include <QDateTime>

static const QString DTC_HISTORY_FILE = QStringLiteral("/root/carputer/dtcs.json");

struct DtcEntry {
    int code;
    const char *system;
    const char *condition;
    const char *areas;
};

static const DtcEntry dtcTable[] = {
    {12, "RPM Signal",        "No G1/G2/NE signal for 2s after STA ON",
                               "G circuit, NE/STA circuit, Distributor, ECM"},
    {13, "RPM Signal",        "No NE signal at >1000rpm for 0.1s",
                               "NE circuit, Distributor, ECM"},
    {14, "Ignition Signal",   "No IGF from igniter for 8-11 consecutive ignition",
                               "IGF/IGT circuit, Igniter, ECM"},
    {21, "O2 Sensor Signal",  "O2 heater circuit open/short for 0.5s (HT)",
                               "O2 heater circuit, O2 sensor, ECM"},
    {22, "Coolant Temp Sens", "THW open/short for 0.5s",
                               "Coolant temp sensor circuit, Sensor, ECM"},
    {24, "Intake Air Temp",   "THA open/short for 0.5s",
                               "Intake air temp sensor circuit, Sensor, ECM"},
    {25, "Air-Fuel Lean",     "O2 <0.45V for 90s at 1500rpm or signal flat",
                               "Engine ground, Injector, O2 sensor, VAF, ECM"},
    {26, "Air-Fuel Rich",     "O2 oscillates >15x/4s at idle or fuel comp off",
                               "Engine ground, Injector leak, O2 sensor, VAF, ECM"},
    {31, "VAF Meter",         "VC circuit open/short at idle for 0.5s",
                               "VAF meter circuit, VAF meter, ECM"},
    {32, "VAF Meter",         "E2/VS circuit open/short at idle for 0.5s",
                               "VAF meter circuit, VAF meter, ECM"},
    {34, "Turbo Pressure",    "PIM open/short for 0.5s or over-boost",
                               "Turbo pressure sensor, Turbo, ECM"},
    {41, "Throttle Position", "VTA open/short for 0.5s, IDL on with VTA>1.5V",
                               "TPS circuit, TPS, ECM"},
    {42, "Vehicle Speed",     "No SPD for 8s at 2500-5000rpm under load",
                               "VSS circuit, VSS, ECM"},
    {43, "Starter Signal",    "No STA until 800rpm reached during crank",
                               "STA circuit, IG SW, Main relay, ECM"},
    {51, "Switch Condition",  "A/C on or IDL off during test mode",
                               "A/C switch, TPS IDL, Accelerator pedal, ECM"},
    {52, "Knock Sensor",      "No KNK signal for 2 revs at 1600-7200rpm",
                               "Knock sensor circuit, Knock sensor, ECM"},
    {53, "Knock Control",     "ECM knock control malfunction",
                               "ECM"},
    {71, "EGR System",        "EGR gas <80°C for 120s with coolant ≥80°C",
                               "EGR temp sensor, VSV circuit, EGR valve/passage, ECM"},
    {0,  nullptr,             nullptr, nullptr}
};

DtcManager::DtcManager(QObject *parent)
    : QObject(parent)
{
    m_socket = new QUdpSocket(this);
    bool bound = m_socket->bind(QHostAddress::Any, 5003, QUdpSocket::ShareAddress);
    if (bound) {
        qDebug() << "DtcManager: listening on UDP 5003 for DTC results";
    } else {
        qWarning() << "DtcManager: failed to bind UDP 5003";
        setStatus("Failed to bind UDP 5003");
    }
    connect(m_socket, &QUdpSocket::readyRead, this, &DtcManager::onReadyRead);

    m_timeoutTimer = new QTimer(this);
    m_timeoutTimer->setSingleShot(true);
    m_timeoutTimer->setInterval(20000);
    connect(m_timeoutTimer, &QTimer::timeout, this, [this]() {
        setBusy(false);
        setStatus("Timeout — no response from sensor module (check wiring)");
        qWarning() << "DtcManager: timeout waiting for DTC response";
    });

    loadHistory();
}

DtcManager::~DtcManager()
{
}

void DtcManager::sendCommand(const QString &cmd)
{
    QJsonObject obj;
    obj["cmd"] = cmd;
    QByteArray data = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    data.append('\n');

    m_socket->writeDatagram(data, QHostAddress("192.168.4.20"), 5002);
    qDebug() << "DtcManager: sent" << cmd << "to 192.168.4.20:5002";
}

void DtcManager::scanDtc()
{
    if (m_busy) return;
    m_lastScanMode = "normal";
    setBusy(true);
    setStatus("Reading DTCs (normal mode)...");
    m_dtcCodes.clear();
    m_dtcCount = 0;
    emit dtcUpdated();
    sendCommand("read_dtc");
    m_timeoutTimer->start();
}

void DtcManager::scanDtcTestMode()
{
    if (m_busy) return;
    m_lastScanMode = "test";
    setBusy(true);
    setStatus("Reading DTCs (test mode — drive >6mph first)...");
    m_dtcCodes.clear();
    m_dtcCount = 0;
    emit dtcUpdated();
    sendCommand("read_dtc_test");
    m_timeoutTimer->start();
}

void DtcManager::onReadyRead()
{
    while (m_socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = m_socket->receiveDatagram();
        QByteArray data = datagram.data().trimmed();
        if (data.isEmpty()) continue;

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(data, &err);
        if (err.error != QJsonParseError::NoError) {
            qWarning() << "DtcManager: JSON parse error" << err.errorString();
            continue;
        }

        QJsonObject obj = doc.object();
        if (obj.value("event").toString() != "dtc") continue;

        m_timeoutTimer->stop();
        m_dtcCodes.clear();
        QString status = obj.value("status").toString();

        if (status == "no_codes") {
            setStatus("No trouble codes found");
            m_dtcCount = 0;
        } else {
            QJsonArray codes = obj.value("codes").toArray();
            for (const QJsonValue &v : codes) {
                m_dtcCodes.append(v.toInt());
            }
            m_dtcCount = m_dtcCodes.size();
            if (m_dtcCount == 0) {
                setStatus("No trouble codes found");
            } else {
                setStatus(QString("Found %1 code(s)").arg(m_dtcCount));
            }
        }

        // Store each code in history with timestamp
        QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        for (int i = 0; i < m_dtcCodes.size(); i++) {
            int code = m_dtcCodes[i].toInt();
            // Avoid duplicating the exact same code within 1 minute
            bool recent = false;
            for (int j = m_dtcHistory.size() - 1; j >= qMax(0, m_dtcHistory.size() - 5); j--) {
                QVariantMap entry = m_dtcHistory[j].toMap();
                if (entry.value("code").toInt() == code &&
                    entry.value("timestamp").toString() == ts) {
                    recent = true;
                    break;
                }
            }
            if (!recent) {
                QVariantMap entry;
                entry["code"] = code;
                entry["timestamp"] = ts;
                entry["description"] = describeCode(code);
                m_dtcHistory.append(entry);
            }
        }
        emit dtcHistoryChanged();
        saveHistory();

        setBusy(false);
        emit dtcUpdated();
        qDebug() << "DtcManager: received" << m_dtcCount << "DTCs";
    }
}

QString DtcManager::describeCode(int code) const
{
    for (int i = 0; dtcTable[i].code != 0; i++) {
        if (dtcTable[i].code == code) {
            return QString("Code %1  %2\nCondition: %3\nCheck: %4")
                .arg(code)
                .arg(dtcTable[i].system)
                .arg(dtcTable[i].condition)
                .arg(dtcTable[i].areas);
        }
    }
    return QString("Code %1 — Unknown").arg(code);
}

void DtcManager::clearHistory()
{
    m_dtcHistory.clear();
    emit dtcHistoryChanged();
    saveHistory();
}

void DtcManager::loadHistory()
{
    QFile file(DTC_HISTORY_FILE);
    if (!file.open(QIODevice::ReadOnly))
        return;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    if (!doc.isArray()) return;
    QJsonArray arr = doc.array();
    for (const QJsonValue &v : arr) {
        QJsonObject o = v.toObject();
        QVariantMap entry;
        entry["code"] = o.value("code").toInt();
        entry["timestamp"] = o.value("timestamp").toString();
        entry["description"] = o.value("description").toString();
        m_dtcHistory.append(entry);
    }
    emit dtcHistoryChanged();
}

void DtcManager::saveHistory()
{
    QDir().mkpath(QStringLiteral("/root/carputer"));
    QFile file(DTC_HISTORY_FILE);
    if (!file.open(QIODevice::WriteOnly))
        return;
    QJsonArray arr;
    for (const QVariant &v : m_dtcHistory) {
        QVariantMap entry = v.toMap();
        QJsonObject o;
        o["code"] = entry.value("code").toInt();
        o["timestamp"] = entry.value("timestamp").toString();
        o["description"] = entry.value("description").toString();
        arr.append(o);
    }
    file.write(QJsonDocument(arr).toJson());
    file.close();
}

void DtcManager::setBusy(bool b)
{
    if (m_busy != b) {
        m_busy = b;
        emit busyChanged();
    }
}

void DtcManager::setStatus(const QString &text)
{
    if (m_statusText != text) {
        m_statusText = text;
        emit statusTextChanged();
    }
}
