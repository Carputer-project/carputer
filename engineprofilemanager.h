#ifndef ENGINEPROFILEMANAGER_H
#define ENGINEPROFILEMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QStringList>

class EngineProfileManager : public QObject
{
    Q_OBJECT

    // Profile identity
    Q_PROPERTY(QString engineCode   READ engineCode   NOTIFY profileChanged)
    Q_PROPERTY(QString engineName   READ engineName   NOTIFY profileChanged)
    Q_PROPERTY(int  displacementCC  READ displacementCC  NOTIFY profileChanged)
    Q_PROPERTY(double compressionRatio READ compressionRatio NOTIFY profileChanged)

    // RPM limits
    Q_PROPERTY(int redlineRPM       READ redlineRPM    NOTIFY profileChanged)
    Q_PROPERTY(int fuelCutRPM       READ fuelCutRPM    NOTIFY profileChanged)
    Q_PROPERTY(int idleRPM          READ idleRPM       NOTIFY profileChanged)
    Q_PROPERTY(int rpmGaugeMax      READ rpmGaugeMax   NOTIFY profileChanged)

    // Coolant thresholds (°F)
    Q_PROPERTY(int coolantGaugeMin   READ coolantGaugeMin  NOTIFY profileChanged)
    Q_PROPERTY(int coolantGaugeMax   READ coolantGaugeMax  NOTIFY profileChanged)
    Q_PROPERTY(int coolantCautionF   READ coolantCautionF  NOTIFY profileChanged)
    Q_PROPERTY(int coolantDangerF    READ coolantDangerF   NOTIFY profileChanged)
    Q_PROPERTY(int coolantCriticalF  READ coolantCriticalF NOTIFY profileChanged)
    Q_PROPERTY(int coolantFanOnLowF  READ coolantFanOnLowF NOTIFY profileChanged)
    Q_PROPERTY(int coolantFanOffLowF READ coolantFanOffLowF NOTIFY profileChanged)
    Q_PROPERTY(int coolantFanOnHighF READ coolantFanOnHighF NOTIFY profileChanged)
    Q_PROPERTY(int coolantFanOffHighF READ coolantFanOffHighF NOTIFY profileChanged)
    Q_PROPERTY(int coolantHealthOptF  READ coolantHealthOptF NOTIFY profileChanged)
    Q_PROPERTY(int coolantHealthCritF READ coolantHealthCritF NOTIFY profileChanged)

    // Oil temperature thresholds (°F)
    Q_PROPERTY(int oilTempGaugeMin   READ oilTempGaugeMin  NOTIFY profileChanged)
    Q_PROPERTY(int oilTempGaugeMax   READ oilTempGaugeMax  NOTIFY profileChanged)
    Q_PROPERTY(int oilTempCautionF   READ oilTempCautionF  NOTIFY profileChanged)
    Q_PROPERTY(int oilTempDangerF    READ oilTempDangerF   NOTIFY profileChanged)
    Q_PROPERTY(int oilTempCriticalF  READ oilTempCriticalF NOTIFY profileChanged)
    Q_PROPERTY(int oilTempHealthOptF READ oilTempHealthOptF NOTIFY profileChanged)
    Q_PROPERTY(int oilTempHealthCritF READ oilTempHealthCritF NOTIFY profileChanged)

    // Oil pressure thresholds (%)
    Q_PROPERTY(int oilPressureGaugeMin  READ oilPressureGaugeMin NOTIFY profileChanged)
    Q_PROPERTY(int oilPressureGaugeMax  READ oilPressureGaugeMax NOTIFY profileChanged)
    Q_PROPERTY(int oilPressureCaution   READ oilPressureCaution  NOTIFY profileChanged)
    Q_PROPERTY(int oilPressureDanger    READ oilPressureDanger   NOTIFY profileChanged)
    Q_PROPERTY(int oilPressureCritical  READ oilPressureCritical NOTIFY profileChanged)
    Q_PROPERTY(int oilPressureHealthOpt READ oilPressureHealthOpt NOTIFY profileChanged)
    Q_PROPERTY(int oilPressureHealthCrit READ oilPressureHealthCrit NOTIFY profileChanged)

    // Battery thresholds (%)
    Q_PROPERTY(int batteryCaution   READ batteryCaution   NOTIFY profileChanged)
    Q_PROPERTY(int batteryDanger    READ batteryDanger    NOTIFY profileChanged)
    Q_PROPERTY(int batteryCritical  READ batteryCritical  NOTIFY profileChanged)
    Q_PROPERTY(int batteryHealthOpt READ batteryHealthOpt NOTIFY profileChanged)
    Q_PROPERTY(int batteryHealthCrit READ batteryHealthCrit NOTIFY profileChanged)

    // Speed gauge (mph)
    Q_PROPERTY(int speedGaugeMaxMPH READ speedGaugeMaxMPH NOTIFY profileChanged)
    Q_PROPERTY(int speedWarnMPH     READ speedWarnMPH     NOTIFY profileChanged)
    Q_PROPERTY(int speedDangerMPH   READ speedDangerMPH   NOTIFY profileChanged)

    // AFR targets
    Q_PROPERTY(double targetAFRIdle   READ targetAFRIdle   NOTIFY profileChanged)
    Q_PROPERTY(double targetAFRCruise READ targetAFRCruise NOTIFY profileChanged)
    Q_PROPERTY(double targetAFRWOT    READ targetAFRWOT    NOTIFY profileChanged)
    Q_PROPERTY(double stoichAFR       READ stoichAFR       NOTIFY profileChanged)
    Q_PROPERTY(double maxFuelCorrection READ maxFuelCorrection NOTIFY profileChanged)
    Q_PROPERTY(double afrDangerLean   READ afrDangerLean   NOTIFY profileChanged)
    Q_PROPERTY(double afrDangerRich   READ afrDangerRich   NOTIFY profileChanged)

    // Shift advice
    Q_PROPERTY(int shiftUpRPM    READ shiftUpRPM    NOTIFY profileChanged)
    Q_PROPERTY(int shiftDownRPM  READ shiftDownRPM  NOTIFY profileChanged)

    // Engine running threshold
    Q_PROPERTY(int engineRunningThreshold READ engineRunningThreshold NOTIFY profileChanged)

    // Health score weights
    Q_PROPERTY(double healthWeightCoolant    READ healthWeightCoolant    NOTIFY profileChanged)
    Q_PROPERTY(double healthWeightOilTemp    READ healthWeightOilTemp    NOTIFY profileChanged)
    Q_PROPERTY(double healthWeightOilPressure READ healthWeightOilPressure NOTIFY profileChanged)
    Q_PROPERTY(double healthWeightBattery    READ healthWeightBattery    NOTIFY profileChanged)
    Q_PROPERTY(double healthWeightDrive      READ healthWeightDrive      NOTIFY profileChanged)

    // Gear ratios
    Q_PROPERTY(QVariantList gearRatios READ gearRatios NOTIFY profileChanged)

    // Injector info
    Q_PROPERTY(int injectorCCMin    READ injectorCCMin    NOTIFY profileChanged)
    Q_PROPERTY(int fuelPressureKPA  READ fuelPressureKPA  NOTIFY profileChanged)

    // Profile management
    Q_PROPERTY(QStringList profileList READ profileList CONSTANT)
    Q_PROPERTY(QString profileName READ profileName NOTIFY profileChanged)

public:
    explicit EngineProfileManager(QObject *parent = nullptr);

    QString engineCode()   const;
    QString engineName()   const;
    int  displacementCC()  const;
    double compressionRatio() const;

    int redlineRPM()       const;
    int fuelCutRPM()       const;
    int idleRPM()          const;
    int rpmGaugeMax()      const;

    int coolantGaugeMin()   const;
    int coolantGaugeMax()   const;
    int coolantCautionF()   const;
    int coolantDangerF()    const;
    int coolantCriticalF()  const;
    int coolantFanOnLowF()  const;
    int coolantFanOffLowF() const;
    int coolantFanOnHighF() const;
    int coolantFanOffHighF() const;
    int coolantHealthOptF()  const;
    int coolantHealthCritF() const;

    int oilTempGaugeMin()   const;
    int oilTempGaugeMax()   const;
    int oilTempCautionF()   const;
    int oilTempDangerF()    const;
    int oilTempCriticalF()  const;
    int oilTempHealthOptF()  const;
    int oilTempHealthCritF() const;

    int oilPressureGaugeMin()  const;
    int oilPressureGaugeMax()  const;
    int oilPressureCaution()   const;
    int oilPressureDanger()    const;
    int oilPressureCritical()  const;
    int oilPressureHealthOpt()  const;
    int oilPressureHealthCrit() const;

    int batteryCaution()   const;
    int batteryDanger()    const;
    int batteryCritical()  const;
    int batteryHealthOpt()  const;
    int batteryHealthCrit() const;

    int speedGaugeMaxMPH() const;
    int speedWarnMPH()     const;
    int speedDangerMPH()   const;

    double targetAFRIdle()   const;
    double targetAFRCruise() const;
    double targetAFRWOT()    const;
    double stoichAFR()       const;
    double maxFuelCorrection() const;
    double afrDangerLean()   const;
    double afrDangerRich()   const;

    int shiftUpRPM()    const;
    int shiftDownRPM()  const;
    int engineRunningThreshold() const;

    double healthWeightCoolant()    const;
    double healthWeightOilTemp()    const;
    double healthWeightOilPressure() const;
    double healthWeightBattery()    const;
    double healthWeightDrive()      const;

    QVariantList gearRatios() const;

    int injectorCCMin()   const;
    int fuelPressureKPA() const;

    QStringList profileList() const;
    QString profileName() const;

    Q_INVOKABLE bool loadProfile(const QString &name);
    Q_INVOKABLE QVariantList availableProfiles() const;  // returns list of {name, code}

signals:
    void profileChanged();

private:
    QJsonObject m_profile;
    QString m_profileName;
    QString m_profileDir;

    void scanProfiles();
    int jsonInt(const QStringList &path, int defaultValue) const;
    double jsonDouble(const QStringList &path, double defaultValue) const;
    QString jsonString(const QStringList &path, const QString &defaultValue) const;
    QJsonObject jsonObject(const QStringList &path) const;
    QJsonArray jsonArray(const QStringList &path) const;

    QStringList m_profileNames;
    QJsonObject m_defaultProfile;
};

#endif // ENGINEPROFILEMANAGER_H
