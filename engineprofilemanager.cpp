#include "engineprofilemanager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QLoggingCategory>
#include <QDebug>
#include <QtMath>

Q_LOGGING_CATEGORY(lcProfile, "carputer.profile")

EngineProfileManager::EngineProfileManager(QObject *parent)
    : QObject(parent)
{
    QStringList searchPaths;
    searchPaths << QCoreApplication::applicationDirPath() + "/engine_profiles/"
                << QDir::currentPath() + "/engine_profiles/"
                << QStringLiteral("/etc/carputer/engine_profiles/");

    for (const QString &path : searchPaths) {
        QDir dir(path);
        if (dir.exists()) {
            m_profileDir = dir.absolutePath();
            break;
        }
    }

    if (m_profileDir.isEmpty()) {
        qCWarning(lcProfile) << "No engine_profiles directory found";
        return;
    }

    // Set up drivetrain.conf path (sibling to engine_profiles/)
    m_drivetrainConfPath = QFileInfo(m_profileDir).dir().absolutePath() + "/drivetrain.conf";
    loadDrivetrainConf();

    scanProfiles();

    // Read current selection
    QString selected;
    QStringList confPaths;
    confPaths << QCoreApplication::applicationDirPath() + "/engine_profile.conf"
              << QDir::currentPath() + "/engine_profile.conf"
              << QStringLiteral("/etc/carputer/engine_profile.conf");
    for (const QString &p : confPaths) {
        QFile f(p);
        if (f.open(QIODevice::ReadOnly)) {
            selected = QString::fromUtf8(f.readAll()).trimmed();
            f.close();
            break;
        }
    }

    if (!selected.isEmpty() && m_profileNames.contains(selected))
        loadProfile(selected);
    else if (!m_profileNames.isEmpty())
        loadProfile(m_profileNames.first());
}

void EngineProfileManager::scanProfiles()
{
    m_profileNames.clear();
    QDir dir(m_profileDir);
    QStringList filters;
    filters << "*.json";
    for (const QFileInfo &fi : dir.entryInfoList(filters, QDir::Files, QDir::Name)) {
        QString code = fi.baseName();
        m_profileNames.append(code);
    }
    qCDebug(lcProfile) << "Found profiles:" << m_profileNames;
}

bool EngineProfileManager::loadProfile(const QString &name)
{
    QString filePath = m_profileDir + "/" + name + ".json";
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(lcProfile) << "Cannot open profile:" << filePath;
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) {
        qCWarning(lcProfile) << "Invalid JSON in profile:" << filePath;
        return false;
    }

    m_profile = doc.object();
    m_profileName = name;
    qCDebug(lcProfile) << "Loaded profile:" << name << "(" << engineCode() << ")";
    emit profileChanged();
    return true;
}

QVariantList EngineProfileManager::availableProfiles() const
{
    QVariantList list;
    for (const QString &name : m_profileNames) {
        QVariantMap entry;
        entry["name"] = name;
        // Try to get full engine code from the profile file
        QString filePath = m_profileDir + "/" + name + ".json";
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            file.close();
            if (doc.isObject()) {
                QJsonObject obj = doc.object();
                entry["code"] = obj["engine"].toObject()["code"].toString(name);
            }
        }
        list.append(entry);
    }
    return list;
}

// ── JSON helper ──────────────────────────────────────────────────────────────

int EngineProfileManager::jsonInt(const QStringList &path, int defaultValue) const
{
    QJsonObject obj = m_profile;
    for (int i = 0; i < path.size() - 1; ++i) {
        if (!obj.contains(path[i])) return defaultValue;
        obj = obj[path[i]].toObject();
    }
    const QString &key = path.last();
    if (!obj.contains(key)) return defaultValue;
    return obj[key].toInt(defaultValue);
}

double EngineProfileManager::jsonDouble(const QStringList &path, double defaultValue) const
{
    QJsonObject obj = m_profile;
    for (int i = 0; i < path.size() - 1; ++i) {
        if (!obj.contains(path[i])) return defaultValue;
        obj = obj[path[i]].toObject();
    }
    const QString &key = path.last();
    if (!obj.contains(key)) return defaultValue;
    return obj[key].toDouble(defaultValue);
}

QString EngineProfileManager::jsonString(const QStringList &path, const QString &defaultValue) const
{
    QJsonObject obj = m_profile;
    for (int i = 0; i < path.size() - 1; ++i) {
        if (!obj.contains(path[i])) return defaultValue;
        obj = obj[path[i]].toObject();
    }
    const QString &key = path.last();
    if (!obj.contains(key)) return defaultValue;
    return obj[key].toString(defaultValue);
}

QJsonObject EngineProfileManager::jsonObject(const QStringList &path) const
{
    QJsonObject obj = m_profile;
    for (int i = 0; i < path.size(); ++i) {
        if (!obj.contains(path[i])) return {};
        obj = obj[path[i]].toObject();
    }
    return obj;
}

QJsonArray EngineProfileManager::jsonArray(const QStringList &path) const
{
    QJsonObject obj = m_profile;
    for (int i = 0; i < path.size() - 1; ++i) {
        if (!obj.contains(path[i])) return {};
        obj = obj[path[i]].toObject();
    }
    const QString &key = path.last();
    if (!obj.contains(key)) return {};
    return obj[key].toArray();
}

// ── Profile identity ─────────────────────────────────────────────────────────

QString EngineProfileManager::engineCode()   const
    { return jsonString({"engine", "code"}, "Generic"); }
QString EngineProfileManager::engineName()   const
    { return jsonString({"engine", "name"}, "Generic 4-Cylinder"); }
int  EngineProfileManager::displacementCC()  const
    { return jsonInt({"engine", "displacement_cc"}, 2000); }
double EngineProfileManager::compressionRatio() const
    { return jsonDouble({"engine", "compression_ratio"}, 9.5); }

// ── RPM ──────────────────────────────────────────────────────────────────────

int EngineProfileManager::redlineRPM()       const
    { return jsonInt({"limits", "rpm", "redline"}, 6500); }
int EngineProfileManager::fuelCutRPM()       const
    { return jsonInt({"limits", "rpm", "fuel_cut"}, 7500); }
int EngineProfileManager::idleRPM()          const
    { return jsonInt({"limits", "rpm", "idle"}, 700); }
int EngineProfileManager::rpmGaugeMax()      const
    { return jsonInt({"limits", "rpm", "gauge_max"}, 8000); }

// ── Coolant ──────────────────────────────────────────────────────────────────

int EngineProfileManager::coolantGaugeMin()   const
    { return jsonInt({"limits", "coolant", "gauge_min"}, 100); }
int EngineProfileManager::coolantGaugeMax()   const
    { return jsonInt({"limits", "coolant", "gauge_max"}, 280); }
int EngineProfileManager::coolantCautionF()   const
    { return jsonInt({"limits", "coolant", "caution_f"}, 220); }
int EngineProfileManager::coolantDangerF()    const
    { return jsonInt({"limits", "coolant", "danger_f"}, 230); }
int EngineProfileManager::coolantCriticalF()  const
    { return jsonInt({"limits", "coolant", "critical_f"}, 245); }
int EngineProfileManager::coolantFanOnLowF()  const
    { return jsonInt({"limits", "coolant", "fan_on_low_f"}, 185); }
int EngineProfileManager::coolantFanOffLowF() const
    { return jsonInt({"limits", "coolant", "fan_off_low_f"}, 179); }
int EngineProfileManager::coolantFanOnHighF() const
    { return jsonInt({"limits", "coolant", "fan_on_high_f"}, 203); }
int EngineProfileManager::coolantFanOffHighF() const
    { return jsonInt({"limits", "coolant", "fan_off_high_f"}, 198); }
int EngineProfileManager::coolantHealthOptF()  const
    { return jsonInt({"limits", "coolant", "health_opt_f"}, 220); }
int EngineProfileManager::coolantHealthCritF() const
    { return jsonInt({"limits", "coolant", "health_crit_f"}, 260); }

// ── Oil temperature ──────────────────────────────────────────────────────────

int EngineProfileManager::oilTempGaugeMin()   const
    { return jsonInt({"limits", "oil_temp", "gauge_min"}, 100); }
int EngineProfileManager::oilTempGaugeMax()   const
    { return jsonInt({"limits", "oil_temp", "gauge_max"}, 300); }
int EngineProfileManager::oilTempCautionF()   const
    { return jsonInt({"limits", "oil_temp", "caution_f"}, 220); }
int EngineProfileManager::oilTempDangerF()    const
    { return jsonInt({"limits", "oil_temp", "danger_f"}, 240); }
int EngineProfileManager::oilTempCriticalF()  const
    { return jsonInt({"limits", "oil_temp", "critical_f"}, 260); }
int EngineProfileManager::oilTempHealthOptF()  const
    { return jsonInt({"limits", "oil_temp", "health_opt_f"}, 220); }
int EngineProfileManager::oilTempHealthCritF() const
    { return jsonInt({"limits", "oil_temp", "health_crit_f"}, 270); }

// ── Oil pressure ─────────────────────────────────────────────────────────────

int EngineProfileManager::oilPressureGaugeMin()  const
    { return jsonInt({"limits", "oil_pressure", "gauge_min"}, 0); }
int EngineProfileManager::oilPressureGaugeMax()  const
    { return jsonInt({"limits", "oil_pressure", "gauge_max"}, 100); }
int EngineProfileManager::oilPressureCaution()   const
    { return jsonInt({"limits", "oil_pressure", "caution"}, 40); }
int EngineProfileManager::oilPressureDanger()    const
    { return jsonInt({"limits", "oil_pressure", "danger"}, 20); }
int EngineProfileManager::oilPressureCritical()  const
    { return jsonInt({"limits", "oil_pressure", "critical"}, 10); }
int EngineProfileManager::oilPressureHealthOpt()  const
    { return jsonInt({"limits", "oil_pressure", "health_opt"}, 40); }
int EngineProfileManager::oilPressureHealthCrit() const
    { return jsonInt({"limits", "oil_pressure", "health_crit"}, 0); }

// ── Battery ──────────────────────────────────────────────────────────────────

int EngineProfileManager::batteryCaution()   const
    { return jsonInt({"limits", "battery", "caution"}, 30); }
int EngineProfileManager::batteryDanger()    const
    { return jsonInt({"limits", "battery", "danger"}, 15); }
int EngineProfileManager::batteryCritical()  const
    { return jsonInt({"limits", "battery", "critical"}, 5); }
int EngineProfileManager::batteryHealthOpt()  const
    { return jsonInt({"limits", "battery", "health_opt"}, 50); }
int EngineProfileManager::batteryHealthCrit() const
    { return jsonInt({"limits", "battery", "health_crit"}, 0); }

// ── Speed ────────────────────────────────────────────────────────────────────

int EngineProfileManager::speedGaugeMaxMPH() const
    { return jsonInt({"limits", "speed", "gauge_max_mph"}, 120); }
int EngineProfileManager::speedWarnMPH()     const
    { return jsonInt({"limits", "speed", "warn_mph"}, 75); }
int EngineProfileManager::speedDangerMPH()   const
    { return jsonInt({"limits", "speed", "danger_mph"}, 90); }

// ── AFR ──────────────────────────────────────────────────────────────────────

double EngineProfileManager::targetAFRIdle()   const
    { return jsonDouble({"limits", "afr", "target_idle"}, 14.7); }
double EngineProfileManager::targetAFRCruise() const
    { return jsonDouble({"limits", "afr", "target_cruise"}, 14.7); }
double EngineProfileManager::targetAFRWOT()    const
    { return jsonDouble({"limits", "afr", "target_wot"}, 12.8); }
double EngineProfileManager::stoichAFR()       const
    { return jsonDouble({"limits", "afr", "stoich"}, 14.7); }
double EngineProfileManager::maxFuelCorrection() const
    { return jsonDouble({"limits", "afr", "max_fuel_correction"}, 20.0); }
double EngineProfileManager::afrDangerLean()   const
    { return jsonDouble({"limits", "afr", "danger_lean"}, 16.0); }
double EngineProfileManager::afrDangerRich()   const
    { return jsonDouble({"limits", "afr", "danger_rich"}, 10.0); }

// ── Shift advice ─────────────────────────────────────────────────────────────

int EngineProfileManager::shiftUpRPM()    const
    { return jsonInt({"limits", "shift_advice", "up_rpm"}, 3500); }
int EngineProfileManager::shiftDownRPM()  const
    { return jsonInt({"limits", "shift_advice", "down_rpm"}, 1300); }
int EngineProfileManager::engineRunningThreshold() const
    { return jsonInt({"limits", "engine_running_threshold"}, 100); }

// ── Health weights ───────────────────────────────────────────────────────────

double EngineProfileManager::healthWeightCoolant()    const
    { return jsonDouble({"health_weights", "coolant"}, 0.30); }
double EngineProfileManager::healthWeightOilTemp()    const
    { return jsonDouble({"health_weights", "oil_temp"}, 0.20); }
double EngineProfileManager::healthWeightOilPressure() const
    { return jsonDouble({"health_weights", "oil_pressure"}, 0.25); }
double EngineProfileManager::healthWeightBattery()    const
    { return jsonDouble({"health_weights", "battery"}, 0.15); }
double EngineProfileManager::healthWeightDrive()      const
    { return jsonDouble({"health_weights", "drive"}, 0.10); }

// ── Gear ratios ──────────────────────────────────────────────────────────────

QVariantList EngineProfileManager::gearRatios() const
{
    QJsonArray arr = jsonArray({"drivetrain", "gear_ratios"});
    if (arr.isEmpty())
        arr = jsonArray({"gear_ratios"});
    if (arr.isEmpty())
        return {3.54, 1.96, 1.25, 0.95, 0.73};
    QVariantList list;
    for (const QJsonValue &v : arr)
        list.append(v.toDouble());
    return list;
}

// ── Drivetrain ───────────────────────────────────────────────────────────────

double EngineProfileManager::finalDrive() const
{
    double fd = jsonDouble({"drivetrain", "final_drive"}, 0);
    if (fd <= 0) fd = jsonDouble({"final_drive"}, 3.944);
    return fd;
}

double EngineProfileManager::tireCircumferenceM() const
{
    int w  = m_userTireWidth > 0 ? m_userTireWidth : jsonInt({"drivetrain", "tire_width"}, 195);
    int ar = m_userTireAspectRatio > 0 ? m_userTireAspectRatio : jsonInt({"drivetrain", "tire_aspect_ratio"}, 65);
    int rd = m_userTireRimDiameter > 0 ? m_userTireRimDiameter : jsonInt({"drivetrain", "tire_rim_diameter"}, 15);

    double sidewallM = (w * ar / 100.0) / 1000.0;
    double rimM = rd * 0.0254;
    double diameterM = rimM + 2.0 * sidewallM;
    return diameterM * M_PI;
}

int EngineProfileManager::userTireWidth() const
{
    if (m_userTireWidth > 0) return m_userTireWidth;
    return jsonInt({"drivetrain", "tire_width"}, 195);
}

int EngineProfileManager::userTireAspectRatio() const
{
    if (m_userTireAspectRatio > 0) return m_userTireAspectRatio;
    return jsonInt({"drivetrain", "tire_aspect_ratio"}, 65);
}

int EngineProfileManager::userTireRimDiameter() const
{
    if (m_userTireRimDiameter > 0) return m_userTireRimDiameter;
    return jsonInt({"drivetrain", "tire_rim_diameter"}, 15);
}

void EngineProfileManager::setUserTireWidth(int mm)
{
    if (mm < 100 || mm > 400) return;
    m_userTireWidth = mm;
    saveDrivetrainConf();
    emit profileChanged();
}

void EngineProfileManager::setUserTireAspectRatio(int pct)
{
    if (pct < 10 || pct > 100) return;
    m_userTireAspectRatio = pct;
    saveDrivetrainConf();
    emit profileChanged();
}

void EngineProfileManager::setUserTireRimDiameter(int inches)
{
    if (inches < 10 || inches > 24) return;
    m_userTireRimDiameter = inches;
    saveDrivetrainConf();
    emit profileChanged();
}

void EngineProfileManager::loadDrivetrainConf()
{
    QFile f(m_drivetrainConfPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!doc.isObject()) return;
    QJsonObject obj = doc.object();
    m_userTireWidth = obj["tire_width"].toInt(0);
    m_userTireAspectRatio = obj["tire_aspect_ratio"].toInt(0);
    m_userTireRimDiameter = obj["tire_rim_diameter"].toInt(0);
    qCDebug(lcProfile) << "Loaded drivetrain overrides:" << m_userTireWidth << m_userTireAspectRatio << m_userTireRimDiameter;
}

void EngineProfileManager::saveDrivetrainConf()
{
    QJsonObject obj;
    obj["tire_width"] = m_userTireWidth;
    obj["tire_aspect_ratio"] = m_userTireAspectRatio;
    obj["tire_rim_diameter"] = m_userTireRimDiameter;
    QFile f(m_drivetrainConfPath);
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        f.write(QJsonDocument(obj).toJson());
        f.close();
        qCDebug(lcProfile) << "Saved drivetrain overrides:" << m_userTireWidth << m_userTireAspectRatio << m_userTireRimDiameter;
    }
}

// ── Injector / Fuel ──────────────────────────────────────────────────────────

int EngineProfileManager::injectorCCMin()   const
    { return jsonInt({"limits", "injector_cc_min"}, 185); }
int EngineProfileManager::fuelPressureKPA() const
    { return jsonInt({"limits", "fuel_pressure_kpa"}, 284); }

// ── Profile list / name ──────────────────────────────────────────────────────

QStringList EngineProfileManager::profileList() const
    { return m_profileNames; }
QString EngineProfileManager::profileName() const
    { return m_profileName; }
