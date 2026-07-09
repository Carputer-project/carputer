import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: dashPage
    focus: true

    property int fanSpeed: carControlManager ? carControlManager.fanSpeed : 0
    property bool hvacOn: carControlManager ? carControlManager.hvacEnabled : configManager.hvacEnabled
    property bool acOn: configManager.acEnabled
    property bool doorsLocked: carControlManager ? carControlManager.doorsLocked : false
    property bool remoteStartActive: carControlManager ? carControlManager.remoteStartActive : false
    property int fanRelay: carControlManager ? carControlManager.fanRelay : 0
    property int radioVolume: carControlManager ? carControlManager.radioVolume : 0
    property bool radioMuted: carControlManager ? carControlManager.radioMuted : true
    property real temperature: configManager.targetTemp
    property bool autoMode: configManager.autoMode
    property bool recirculate: configManager.recirculate
    property int centerView: 0
    property int focusIndex: -1

    // ── Engine Health Score ──────────────────────────────────────────────
    property int _redlineRPM: engineProfile ? engineProfile.redlineRPM : 6500
    property int _fuelCutRPM: engineProfile ? engineProfile.fuelCutRPM : 7500
    property int _rpmGaugeMax: engineProfile ? engineProfile.rpmGaugeMax : 8000
    property double _rpmWarnFraction: _rpmGaugeMax > 0 ? Math.min((_redlineRPM - 500) / _rpmGaugeMax, 0.85) : 0.75
    property double _rpmRedlineFraction: _rpmGaugeMax > 0 ? _redlineRPM / _rpmGaugeMax : 0.85
    property double _rpmDangerFraction: _rpmGaugeMax > 0 ? _fuelCutRPM / _rpmGaugeMax : 0.875

    property int _coolantGaugeMin: engineProfile ? engineProfile.coolantGaugeMin : 100
    property int _coolantGaugeMax: engineProfile ? engineProfile.coolantGaugeMax : 280
    property double _coolantWarnFraction: _coolantGaugeMax > _coolantGaugeMin ? (engineProfile.coolantCautionF - _coolantGaugeMin) / (_coolantGaugeMax - _coolantGaugeMin) : 0.67
    property double _coolantDangerFraction: _coolantGaugeMax > _coolantGaugeMin ? (engineProfile.coolantDangerF - _coolantGaugeMin) / (_coolantGaugeMax - _coolantGaugeMin) : 0.78

    property int _speedGaugeMax: engineProfile ? engineProfile.speedGaugeMaxMPH : 120
    property double _speedWarnFraction: engineProfile ? Math.min(engineProfile.speedWarnMPH / _speedGaugeMax, 0.9) : 0.75
    property double _speedDangerFraction: engineProfile ? Math.min(engineProfile.speedDangerMPH / _speedGaugeMax, 0.95) : 0.92

    property int _oilPressCaution: engineProfile ? engineProfile.oilPressureCaution : 40
    property int _oilPressDanger: engineProfile ? engineProfile.oilPressureDanger : 20

    property var _gearRatios: engineProfile ? engineProfile.gearRatios : [110, 65, 42, 30]
    property int _shiftUpRPM: engineProfile ? engineProfile.shiftUpRPM : 3500
    property int _shiftDownRPM: engineProfile ? engineProfile.shiftDownRPM : 1300
    property int _engineRunningThreshold: engineProfile ? engineProfile.engineRunningThreshold : 100

    property int _coolantCautionF: engineProfile ? engineProfile.coolantCautionF : 220
    property int _coolantDangerF: engineProfile ? engineProfile.coolantDangerF : 230
    property int _coolantCriticalF: engineProfile ? engineProfile.coolantCriticalF : 245
    property int _oilTempCautionF: engineProfile ? engineProfile.oilTempCautionF : 220
    property int _oilTempDangerF: engineProfile ? engineProfile.oilTempDangerF : 240
    property int _oilTempCriticalF: engineProfile ? engineProfile.oilTempCriticalF : 260
    property int _oilPressCritical: engineProfile ? engineProfile.oilPressureCritical : 10
    property int _battHealthOpt: engineProfile ? engineProfile.batteryHealthOpt : 50
    property int _battHealthCrit: engineProfile ? engineProfile.batteryHealthCrit : 0

    property double _wCoolant: engineProfile ? engineProfile.healthWeightCoolant : 0.30
    property double _wOilTemp: engineProfile ? engineProfile.healthWeightOilTemp : 0.20
    property double _wOilPress: engineProfile ? engineProfile.healthWeightOilPressure : 0.25
    property double _wBattery: engineProfile ? engineProfile.healthWeightBattery : 0.15
    property double _wDrive: engineProfile ? engineProfile.healthWeightDrive : 0.10

    property int _coolantHealthOpt: engineProfile ? engineProfile.coolantHealthOptF : 220
    property int _coolantHealthCrit: engineProfile ? engineProfile.coolantHealthCritF : 260
    property int _oilTempHealthOpt: engineProfile ? engineProfile.oilTempHealthOptF : 220
    property int _oilTempHealthCrit: engineProfile ? engineProfile.oilTempHealthCritF : 270
    property int _oilPressHealthOpt: engineProfile ? engineProfile.oilPressureHealthOpt : 40
    property int _oilPressHealthCrit: engineProfile ? engineProfile.oilPressureHealthCrit : 0

    property int healthScore: {
        if (!sensorManager) return 0
        var ct = sensorManager.coolantTemp
        var coolantScore = ct <= 0 ? 100 : (ct < _coolantHealthOpt ? 100 : Math.max(0, 100 - (ct - _coolantHealthOpt) * 100 / (_coolantHealthCrit - _coolantHealthOpt)))
        var ot = sensorManager.oilTemp
        var oilScore = ot <= 0 ? 100 : (ot < _oilTempHealthOpt ? 100 : Math.max(0, 100 - (ot - _oilTempHealthOpt) * 100 / (_oilTempHealthCrit - _oilTempHealthOpt)))
        var op = sensorManager.oilPressure
        var pressureScore = op <= 0 ? 100 : (op > _oilPressHealthOpt ? 100 : Math.max(0, op * 100 / _oilPressHealthOpt))
        var bat = sensorManager.battery
        var batteryScore = bat <= 0 ? 100 : (bat > _battHealthOpt ? 100 : Math.max(0, bat * 100 / _battHealthOpt))
        var tps = sensorManager.throttle
        var driveScore = tps < 30 ? 100 : (tps < 60 ? 80 : (tps < 80 ? 60 : 40))
        return Math.round(coolantScore * _wCoolant + oilScore * _wOilTemp + pressureScore * _wOilPress + batteryScore * _wBattery + driveScore * _wDrive)
    }
    property string healthColor: healthScore >= 80 ? themeManager.statusGreen : (healthScore >= 50 ? themeManager.statusYellow : themeManager.statusRed)

    property bool engineRunning: sensorManager ? sensorManager.rpm > _engineRunningThreshold : false

    // ── Shift Indicator ─────────────────────────────────────────────────
    // Approximate gear detection from speed/rpm ratio for 5-speed manual
    property int currentGear: {
        if (!sensorManager || sensorManager.rpm <= 0 || sensorManager.speed <= 0) return 0
        var ratio = sensorManager.rpm / Math.max(1, sensorManager.speed)
        if (ratio > _gearRatios[0]) return 1
        if (ratio > _gearRatios[1]) return 2
        if (ratio > _gearRatios[2]) return 3
        if (ratio > _gearRatios[3]) return 4
        return 5
    }
    property string shiftAdvice: {
        if (!sensorManager || currentGear === 0) return ""
        var rpm = sensorManager.rpm
        if (rpm > _shiftUpRPM && currentGear < 5) return "↑  " + currentGear + "→" + (currentGear + 1)
        if (rpm < _shiftDownRPM && currentGear > 1) return "↓  " + currentGear + "→" + (currentGear - 1)
        return ""
    }
    property bool shouldShift: shiftAdvice.length > 0

    // ── Warning system ──────────────────────────────────────────────────
    property int _lastAlertLevel: 0
    property string coolantAlert: {
        if (!sensorManager) return "ok"
        var t = sensorManager.coolantTemp
        if (t > _coolantCriticalF) return "critical"
        if (t > _coolantDangerF) return "danger"
        if (t > _coolantCautionF) return "caution"
        return "ok"
    }
    property string oilTempAlert: {
        if (!sensorManager) return "ok"
        var t = sensorManager.oilTemp
        if (t > _oilTempCriticalF) return "critical"
        if (t > _oilTempDangerF) return "danger"
        if (t > _oilTempCautionF) return "caution"
        return "ok"
    }
property string oilPressureAlert: {
    if (!sensorManager) return "ok"
    if (!engineRunning) return "ok"  // engine off = no oil pressure warning
    var p = sensorManager.oilPressure
    if (p <= 0) return "ok"
    if (p < _oilPressCritical) return "critical"
    if (p < _oilPressDanger) return "danger"
    if (p < _oilPressCaution) return "caution"
    return "ok"
}
    property string batteryAlert: {
        if (!sensorManager) return "ok"
        var b = sensorManager.battery
        if (b <= 0) return "ok"
        if (b < _battCritical) return "critical"
        if (b < _battDanger) return "danger"
        if (b < _battCaution) return "caution"
        return "ok"
    }
    property string worstAlert: {
        var a = [coolantAlert, oilTempAlert, oilPressureAlert, batteryAlert]
        if (a.indexOf("critical") >= 0) return "critical"
        if (a.indexOf("danger") >= 0) return "danger"
        if (a.indexOf("caution") >= 0) return "caution"
        return "ok"
    }
    property var activeWarnings: {
        var list = []
        if (coolantAlert !== "ok") list.push({sensor:"COOLANT", level:coolantAlert, value:sensorManager?sensorManager.coolantTemp+"°F":"", action: coolantAlert==="critical"?"STOP ENGINE - Pull over immediately":(coolantAlert==="danger"?"Reduce load, check coolant level":"Monitor temperature")})
        if (oilTempAlert !== "ok") list.push({sensor:"OIL TEMP", level:oilTempAlert, value:sensorManager?sensorManager.oilTemp+"°F":"", action: oilTempAlert==="critical"?"STOP ENGINE - Oil breakdown risk":(oilTempAlert==="danger"?"Reduce engine load":"Monitor oil temperature")})
        if (oilPressureAlert !== "ok") list.push({sensor:"OIL PRESS", level:oilPressureAlert, value:sensorManager?sensorManager.oilPressure+"%":"", action: oilPressureAlert==="critical"?"STOP ENGINE - No oil pressure!":(oilPressureAlert==="danger"?"Check oil level immediately":"Check oil level soon")})
        if (batteryAlert !== "ok") list.push({sensor:"BATTERY", level:batteryAlert, value:sensorManager?sensorManager.battery+"%":"", action: batteryAlert==="critical"?"Charge or replace battery":(batteryAlert==="danger"?"Recharge battery soon":"Monitor battery level")})
        return list
    }
    // Track whether we've shown the current set of warnings
    property string _dismissedKey: ""

    onWorstAlertChanged: {
        if (worstAlert !== "ok") _dismissedKey = ""
    }

    Connections {
        target: carControlManager
        function onFanSpeedChanged() { fanSpeed = carControlManager.fanSpeed }
        function onHvacEnabledChanged() { hvacOn = carControlManager.hvacEnabled }
        function onAcEnabledChanged() { acOn = carControlManager.acEnabled }
        function onDoorsLockedChanged() { doorsLocked = carControlManager.doorsLocked }
        function onRemoteStartActiveChanged() { remoteStartActive = carControlManager.remoteStartActive }
        function onRadioChanged() { radioVolume = carControlManager.radioVolume; radioMuted = carControlManager.radioMuted }
    }
    onTemperatureChanged: configManager.targetTemp = temperature

    Image {
        id: dashBackground
        anchors.fill: parent
        source: configManager.backgroundImage.length > 0
                ? "file://" + configManager.backgroundImage
                : ""
        fillMode: Image.PreserveAspectCrop
        asynchronous: true
        cache: false
        opacity: configManager.backgroundOpacity
        visible: source.toString().length > 0
        onStatusChanged: {
            if (status === Image.Error)
                console.log("BG IMAGE ERROR: couldn't load", source)
            else if (status === Image.Ready)
                console.log("BG IMAGE LOADED:", source)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6

        // ── Status Bar ─────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            color: themeManager.bgCard
            radius: 8
            border.width: 1
            border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.25)
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12
                spacing: 8
                // Vehicle status left
                RowLayout {
                    spacing: 6
                    Rectangle {
                        width: 10; height: 10; radius: 5
                        color: doorsLocked ? themeManager.statusGreen : themeManager.statusRed
                    }
                    Text {
                        text: doorsLocked ? "LOCKED" : "UNLOCKED"
                        color: doorsLocked ? themeManager.statusGreen : themeManager.statusRed
                        font.pixelSize: 11; font.bold: true
                    }
                    Rectangle {
                        width: 1; height: 16; color: themeManager.bgPanel
                    }
                    Text {
                        text: remoteStartActive ? "ENGINE ON" : "ENGINE OFF"
                        color: remoteStartActive ? themeManager.carOrange : themeManager.textSecondary
                        font.pixelSize: 11; font.bold: remoteStartActive
                    }
                    Rectangle {
                        width: 1; height: 16; color: themeManager.bgPanel
                        visible: sensorManager
                    }
                    Row {
                        spacing: 2
                        visible: sensorManager
                        Repeater {
                            model: [
                                {label:"D", door: sensorManager ? sensorManager.driverDoor : false},
                                {label:"P", door: sensorManager ? sensorManager.passengerDoor : false},
                                {label:"T", door: sensorManager ? sensorManager.trunk : false},
                                {label:"H", door: sensorManager ? sensorManager.hood : false}
                            ]
                            Rectangle {
                                width: 18; height: 16; radius: 3
                                color: modelData.door ? themeManager.carOrange : themeManager.bgPanel
                                border.width: 1
                                border.color: modelData.door ? themeManager.carOrange : themeManager.textSecondary
                                Text {
                                    anchors.centerIn: parent
                                    text: modelData.label
                                    color: modelData.door ? "#000" : themeManager.textSecondary
                                    font.pixelSize: 9; font.bold: modelData.door
                                }
                            }
                        }
                    }
                }
                Item { Layout.fillWidth: true }
                // Center time
                Text {
                    id: timeText
                    text: new Date().toLocaleTimeString(Qt.locale("en_US"), "hh:mm AP")
                    color: themeManager.textPrimary
                    font.pixelSize: 18; font.bold: true
                }
                Item { Layout.fillWidth: true }
                // Right status - warning indicators
                RowLayout {
                    spacing: 4
                    Repeater {
                        model: [
                            {label:"COOL", alert:dashPage.coolantAlert},
                            {label:"OIL", alert:dashPage.oilTempAlert},
                            {label:"PRESS", alert:dashPage.oilPressureAlert},
                            {label:"BAT", alert:dashPage.batteryAlert}
                        ]
                        Rectangle {
                            width: 28; height: 14; radius: 3
                            color: modelData.alert === "critical" ? themeManager.statusRed :
                                  modelData.alert === "danger" ? themeManager.statusOrange :
                                  modelData.alert === "caution" ? themeManager.statusYellow :
                                  themeManager.bgPanel
                            Text {
                                anchors.centerIn: parent
                                text: modelData.label
                                color: modelData.alert === "ok" ? themeManager.textSecondary : "#000"
                                font.pixelSize: 8; font.bold: modelData.alert !== "ok"
                            }
                        }
                    }
                    // Health score badge
                    Rectangle {
                        width: 8; height: 8; radius: 4
                        color: dashPage.healthColor
                        visible: sensorManager && sensorManager.connected
                    }
                    Text {
                        text: sensorManager && sensorManager.connected ? "HEALTH " + dashPage.healthScore + "%" : "OFFLINE"
                        color: dashPage.healthColor
                        font.pixelSize: 11; font.bold: true
                    }
                    Text {
                        text: "v" + appVersion
                        color: themeManager.textSecondary; font.pixelSize: 10
                    }
                }
            }
        }

        // ── Main Gauge Area ─────────────────────────────────────────────
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            // Speedometer
            AnalogGauge {
                id: speedoGauge
                width: 380; height: 380
                anchors { left: parent.left; leftMargin: 20; verticalCenter: parent.verticalCenter; verticalCenterOffset: -60 }
                value: sensorManager ? sensorManager.speed : 0
                minValue: 0; maxValue: _speedGaugeMax
                label: "SPEED"
                unitLabel: "mph"
                fontSize: 22
                majorTicks: 6
                minorTicks: 4
                warnValue: _speedWarnFraction; dangerValue: _speedDangerFraction
                redlineStart: _speedDangerFraction
                thickness: 0.12
            }

            // RPM Gauge
            AnalogGauge {
                id: rpmGauge
                width: 380; height: 380
                anchors { right: parent.right; rightMargin: 20; verticalCenter: parent.verticalCenter; verticalCenterOffset: -60 }
                value: sensorManager ? sensorManager.rpm : 0
                minValue: 0; maxValue: _rpmGaugeMax
                label: "ENGINE"
                unitLabel: "rpm"
                fontSize: 22
                majorTicks: 8
                minorTicks: 4
                warnValue: _rpmWarnFraction; dangerValue: _rpmDangerFraction
                redlineStart: _rpmRedlineFraction
                thickness: 0.12
            }

            // ── Shift Indicator ───────────────────────────────────────────
            Item {
                anchors {
                    right: rpmGauge.right; rightMargin: 10
                    top: rpmGauge.top; topMargin: 30
                }
                width: 80; height: 36
                visible: shouldShift && sensorManager && sensorManager.connected
                Rectangle {
                    anchors.fill: parent
                    radius: 8
                    color: "#cc000000"
                    border.width: 2
                    border.color: shiftAdvice.indexOf("↑") >= 0 ? themeManager.statusGreen : themeManager.carOrange
                    Text {
                        anchors.centerIn: parent
                        text: dashPage.shiftAdvice
                        color: shiftAdvice.indexOf("↑") >= 0 ? themeManager.statusGreen : themeManager.carOrange
                        font.pixelSize: 20; font.bold: true
                    }
                }
                SequentialAnimation on opacity {
                    loops: Animation.Infinite
                    PropertyAnimation { to: 1.0; duration: 400 }
                    PropertyAnimation { to: 0.3; duration: 400 }
                }
            }

            // Central Info Panel
            Rectangle {
                anchors {
                    left: speedoGauge.right; leftMargin: 12
                    right: rpmGauge.left; rightMargin: 12
                    verticalCenter: parent.verticalCenter; verticalCenterOffset: -60
                }
                height: 420
                color: themeManager.bgCard
                radius: 10
                clip: true
                border.width: dashPage.focusIndex === 14 ? 2 : 1
                border.color: dashPage.focusIndex === 14 ? themeManager.carBlue : Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.28)

                Rectangle {
                    anchors.fill: parent
                    radius: parent.radius
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.08) }
                        GradientStop { position: 0.35; color: "transparent" }
                        GradientStop { position: 1.0; color: "transparent" }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: centerView = (centerView + 1) % 3
                }

                // View 0: Now Playing
                Item {
                    anchors.fill: parent
                    anchors.margins: 16
                    visible: centerView === 0
                    Column {
                        width: parent.width
                        spacing: 8

                        Text {
                            text: "NOW PLAYING"
                            color: themeManager.textSecondary
                            font.pixelSize: 11; font.bold: true
                        }

                        Rectangle {
                            width: parent.width; height: 120; radius: 8
                            color: themeManager.bgDark; clip: true
                            Row {
                                anchors.fill: parent; anchors.margins: 10; spacing: 12
                                Rectangle {
                                    width: 80; height: 80; radius: 6
                                    anchors.verticalCenter: parent.verticalCenter
                                    color: themeManager.bgPanel; clip: true
                                    Image {
                                        id: dashArtwork
                                        anchors.fill: parent
                                        source: mediaManager.artworkUrl.length > 0 ? mediaManager.artworkUrl : ""
                                        fillMode: Image.PreserveAspectCrop
                                        visible: status === Image.Ready
                                        cache: false
                                    }
                                    Text {
                                        anchors.centerIn: parent; text: "♫"
                                        font.pixelSize: 28; color: themeManager.textSecondary
                                        visible: dashArtwork.status !== Image.Ready
                                    }
                                }
                                Column {
                                    anchors.verticalCenter: parent.verticalCenter
                                    width: parent.width - 104; spacing: 4
                                    Text {
                                        text: mediaManager.currentTitle || mediaManager.currentTrack || "No track loaded"
                                        color: themeManager.textPrimary; font.pixelSize: 15; font.bold: true
                                        elide: Text.ElideRight; width: parent.width
                                    }
                                    Text {
                                        text: mediaManager.currentArtist || ""
                                        color: themeManager.carBlue; font.pixelSize: 12
                                        elide: Text.ElideRight; width: parent.width
                                    }
                                    Text {
                                        text: formatTime(mediaManager.position) + " / " + formatTime(mediaManager.duration)
                                        color: themeManager.textSecondary; font.pixelSize: 11
                                    }
                                }
                            }
                        }

                        // Transport controls
                        Row {
                            spacing: 8; anchors.horizontalCenter: parent.horizontalCenter
                            Button { text: "◀◀"; width: 50; highlighted: dashPage.focusIndex === 10; onClicked: mediaManager.previous() }
                            Button {
                                text: mediaManager.playing ? "⏸" : "▶"; width: 70
                                highlighted: dashPage.focusIndex === 11
                                onClicked: mediaManager.playing ? mediaManager.pause() : mediaManager.play()
                            }
                            Button { text: "▶▶"; width: 50; highlighted: dashPage.focusIndex === 12; onClicked: mediaManager.next() }
                        }

                        // Spectrum mini
                        Rectangle {
                            width: parent.width; height: 50; radius: 6; color: themeManager.bgDark; clip: true
                            Row {
                                anchors.fill: parent; anchors.margins: 4; spacing: 1
                                Repeater {
                                    model: mediaManager.spectrumData.length > 0 ? mediaManager.spectrumData : Array(16).fill(-80.0)
                                    Item {
                                        width: (parent.width - 15) / 16; height: parent.height
                                        Rectangle {
                                            property real normalized: Math.max(0.0, (modelData + 80.0) / 80.0)
                                            width: parent.width; height: Math.max(2, normalized * parent.height)
                                            anchors.bottom: parent.bottom; radius: 1
                                            color: normalized < 0.5 ? Qt.rgba(0, 0.66 + normalized * 0.68, 0.91, 1.0) : Qt.rgba(0, 1.0, 0.91 - (normalized - 0.5) * 1.4, 1.0)
                                            Behavior on height { NumberAnimation { duration: 60; easing.type: Easing.OutQuart } }
                                        }
                                    }
                                }
                            }
                        }

                        // Tap hint
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "tap for trip →"
                            color: themeManager.textSecondary; font.pixelSize: 9
                        }
                    }
                }

                // View 1: Trip Computer
                Item {
                    anchors.fill: parent
                    anchors.margins: 16
                    visible: centerView === 1
                    Column {
                        width: parent.width
                        spacing: 6
                        Text {
                            text: "TRIP COMPUTER"
                            color: themeManager.textSecondary
                            font.pixelSize: 11; font.bold: true
                        }

                        // Distance
                        Rectangle {
                            width: parent.width; height: 60; radius: 6; color: themeManager.bgDark
                            Column { anchors.centerIn: parent; spacing: 2
                                Text { anchors.horizontalCenter: parent.horizontalCenter; text: "DISTANCE"; color: themeManager.textSecondary; font.pixelSize: 9 }
                                Text { anchors.horizontalCenter: parent.horizontalCenter; text: tripComputer ? tripComputer.distance.toFixed(1) + " mi" : "--"; color: themeManager.carBlue; font.pixelSize: 24; font.bold: true }
                            }
                        }

                        // Avg Speed + Fuel Used
                        Row {
                            spacing: 8; width: parent.width
                            Rectangle {
                                width: (parent.width - 8) / 2; height: 56; radius: 6; color: themeManager.bgDark
                                Column { anchors.centerIn: parent; spacing: 2
                                    Text { anchors.horizontalCenter: parent.horizontalCenter; text: "AVG SPEED"; color: themeManager.textSecondary; font.pixelSize: 9 }
                                    Text { anchors.horizontalCenter: parent.horizontalCenter; text: tripComputer ? tripComputer.avgSpeed.toFixed(1) + " mph" : "--"; color: themeManager.textPrimary; font.pixelSize: 20; font.bold: true }
                                }
                            }
                            Rectangle {
                                width: (parent.width - 8) / 2; height: 56; radius: 6; color: themeManager.bgDark
                                Column { anchors.centerIn: parent; spacing: 2
                                    Text { anchors.horizontalCenter: parent.horizontalCenter; text: "FUEL USED"; color: themeManager.textSecondary; font.pixelSize: 9 }
                                    Text { anchors.horizontalCenter: parent.horizontalCenter; text: tripComputer ? tripComputer.fuelUsed.toFixed(2) + " gal" : "--"; color: themeManager.carOrange; font.pixelSize: 20; font.bold: true }
                                }
                            }
                        }

                        // Instant MPG + Trip Timer
                        Row {
                            spacing: 8; width: parent.width
                            Rectangle {
                                width: (parent.width - 8) / 2; height: 56; radius: 6; color: themeManager.bgDark
                                Column { anchors.centerIn: parent; spacing: 2
                                    Text { anchors.horizontalCenter: parent.horizontalCenter; text: "INSTANT MPG"; color: themeManager.textSecondary; font.pixelSize: 9 }
                                    Text { anchors.horizontalCenter: parent.horizontalCenter; text: tripComputer ? tripComputer.instantMpg.toFixed(1) : "--"; color: themeManager.statusGreen; font.pixelSize: 20; font.bold: true }
                                }
                            }
                            Rectangle {
                                width: (parent.width - 8) / 2; height: 56; radius: 6; color: themeManager.bgDark
                                Column { anchors.centerIn: parent; spacing: 2
                                    Text { anchors.horizontalCenter: parent.horizontalCenter; text: "TRIP TIME"; color: themeManager.textSecondary; font.pixelSize: 9 }
                                    Text { anchors.horizontalCenter: parent.horizontalCenter; text: tripComputer ? formatTripTime(tripComputer.tripTime) : "--"; color: themeManager.textPrimary; font.pixelSize: 20; font.bold: true }
                                }
                            }
                        }

                        // Reset button
                        Button {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "RESET TRIP"
                            highlighted: dashPage.focusIndex === 13
                            palette.button: themeManager.carOrange
                            onClicked: if (tripComputer) tripComputer.reset()
                        }

                        // Health score card
                        Rectangle {
                            width: parent.width; height: 40; radius: 6; color: themeManager.bgDark
                            border.width: 1
                            border.color: dashPage.healthColor
                            Row {
                                anchors.centerIn: parent; spacing: 8
                                Rectangle { width: 10; height: 10; radius: 5; anchors.verticalCenter: parent.verticalCenter; color: dashPage.healthColor }
                                Text { anchors.verticalCenter: parent.verticalCenter; text: "ENGINE HEALTH: " + dashPage.healthScore + "%"; color: dashPage.healthColor; font.pixelSize: 14; font.bold: true }
                            }
                        }

                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "← tap for performance →"
                            color: themeManager.textSecondary; font.pixelSize: 9
                        }
                    }
                }

                // View 2: Performance
                Column {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 8
                    visible: centerView === 2
                    Text {
                        text: "PERFORMANCE"
                        color: themeManager.textSecondary
                        font.pixelSize: 11; font.bold: true
                    }

                    Column {
                        width: parent.width
                        spacing: 12
                        Text { text: "VEHICLE STATUS"; color: themeManager.textSecondary; font.pixelSize: 10 }
                        Row {
                            spacing: 6; anchors.horizontalCenter: parent.horizontalCenter
                            Rectangle { width: 12; height: 12; radius: 6; color: sensorManager && sensorManager.connected ? themeManager.statusGreen : themeManager.statusRed }
                            Text {
                                text: sensorManager && sensorManager.connected ? "ALL SYSTEMS NOMINAL" : "SENSOR OFFLINE"
                                color: sensorManager && sensorManager.connected ? themeManager.statusGreen : themeManager.statusRed
                                font.pixelSize: 18; font.bold: true
                            }
                        }
                        Rectangle { width: parent.width; height: 1; color: themeManager.bgPanel }
                        Column { spacing: 6; width: parent.width
                            Row { width: parent.width
                                Text { text: "Ambient"; color: themeManager.textSecondary; font.pixelSize: 12; width: parent.width / 2 }
                                Text { text: sensorManager ? sensorManager.ambientTemp + "°F" : "--"; color: themeManager.textPrimary; font.pixelSize: 14; font.bold: true }
                            }
                            Row { width: parent.width
                                Text { text: "Intake"; color: themeManager.textSecondary; font.pixelSize: 12; width: parent.width / 2 }
                                Text { text: sensorManager ? sensorManager.intakeTemp + "°F" : "--"; color: themeManager.textPrimary; font.pixelSize: 14; font.bold: true }
                            }
                            Row { width: parent.width
                                Text { text: "Oil Temp"; color: themeManager.textSecondary; font.pixelSize: 12; width: parent.width / 2 }
                                Text { text: sensorManager ? sensorManager.oilTemp + "°F" : "--"; color: themeManager.textPrimary; font.pixelSize: 14; font.bold: true }
                            }
                            Row { width: parent.width
                                Text { text: "Brake Fluid"; color: themeManager.textSecondary; font.pixelSize: 12; width: parent.width / 2 }
                                Text { text: sensorManager ? sensorManager.brakeFluid + "%" : "--"; color: themeManager.textPrimary; font.pixelSize: 14; font.bold: true }
                            }
                        }
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: "← tap for now playing"
                        color: themeManager.textSecondary; font.pixelSize: 9
                    }
                }
            }

            AnalogGauge {
                id: fuelGauge
                width: 160; height: 120
                anchors { left: speedoGauge.left; bottom: parent.bottom; bottomMargin: 30 }
                value: sensorManager ? sensorManager.fuelLevel : 0
                minValue: 0; maxValue: 100
                label: "FUEL"
                unitLabel: "%"
                fontSize: 11
                majorTicks: 4
                minorTicks: 2
                showValue: true; showNeedle: true
                startAngle: 180; endAngle: 360
                warnValue: 0.25; dangerValue: 0.10
                redlineStart: 1.0
                thickness: 0.22
            }

            AnalogGauge {
                id: tpsGauge
                width: 140; height: 110
                anchors { left: fuelGauge.right; leftMargin: 8; top: fuelGauge.top; topMargin: 5 }
                value: sensorManager ? sensorManager.throttle : 0
                minValue: 0; maxValue: 100
                label: "TPS"
                unitLabel: "%"
                fontSize: 10
                majorTicks: 4
                minorTicks: 2
                showValue: true; showNeedle: true
                startAngle: 180; endAngle: 360
                warnValue: 0.75; dangerValue: 0.90
                redlineStart: 1.0
                thickness: 0.22
            }

            AnalogGauge {
                id: mapGauge
                width: 140; height: 110
                anchors { left: tpsGauge.right; leftMargin: 8; top: tpsGauge.top }
                value: sensorManager ? sensorManager.map : 0
                minValue: 0; maxValue: 100
                label: "MAP"
                unitLabel: "%"
                fontSize: 10
                majorTicks: 4
                minorTicks: 2
                showValue: true; showNeedle: true
                startAngle: 180; endAngle: 360
                warnValue: 0.75; dangerValue: 0.90
                redlineStart: 1.0
                thickness: 0.22
            }

            AnalogGauge {
                id: oilPressGauge
                width: 140; height: 110
                anchors { left: mapGauge.right; leftMargin: 8; top: tpsGauge.top }
                value: sensorManager ? sensorManager.oilPressure : 0
                minValue: 0; maxValue: 100
                label: "OIL P"
                unitLabel: "%"
                fontSize: 10
                majorTicks: 4
                minorTicks: 2
                showValue: true; showNeedle: true
                startAngle: 180; endAngle: 360
                warnValue: _oilPressCaution / 100.0; dangerValue: _oilPressDanger / 100.0
                redlineStart: 1.0
                thickness: 0.22
            }

            // Wideband O2 gauge (piggyback module)
            WidebandGauge {
                id: wboGauge
                width: 100; height: 100
                anchors { left: oilPressGauge.right; leftMargin: 6; top: tpsGauge.top; topMargin: 0 }
                afr: sensorManager ? sensorManager.wboAFR : 14.7
                lambda: sensorManager ? sensorManager.wboLambda : 1.0
                targetAfr: sensorManager ? sensorManager.targetAFR : 14.7
                correction: sensorManager ? sensorManager.fuelCorrection : 0.0
                opacity: sensorManager ? 1.0 : 0.3
                Behavior on opacity { NumberAnimation { duration: 300 } }
            }

            AnalogGauge {
                id: tempGauge
                width: 160; height: 120
                anchors { right: rpmGauge.right; bottom: parent.bottom; bottomMargin: 30 }
                value: sensorManager ? sensorManager.coolantTemp : 0
                minValue: _coolantGaugeMin; maxValue: _coolantGaugeMax
                label: "COOLANT"
                unitLabel: "°F"
                fontSize: 11
                majorTicks: 4
                minorTicks: 2
                showValue: true; showNeedle: true
                startAngle: 180; endAngle: 360
                warnValue: _coolantWarnFraction; dangerValue: _coolantDangerFraction
                redlineStart: _coolantWarnFraction
                thickness: 0.22
            }

            // Fan status indicator
            Column {
                anchors { verticalCenter: tempGauge.verticalCenter; right: tempGauge.left; rightMargin: 6 }
                spacing: 6

                Rectangle {
                    width: 28; height: 28; radius: 14
                    border.width: 2
                    border.color: dashPage.fanRelay >= 1 ? themeManager.carBlue : themeManager.textSecondary
                    color: dashPage.fanRelay >= 1 ? themeManager.carBlue : "transparent"
                    Text {
                        anchors.centerIn: parent
                        text: "1"
                        font.pixelSize: 13; font.bold: true
                        color: dashPage.fanRelay >= 1 ? "#ffffff" : themeManager.textSecondary
                    }
                }

                Rectangle {
                    width: 28; height: 28; radius: 14
                    border.width: 2
                    border.color: dashPage.fanRelay >= 2 ? themeManager.carBlue : themeManager.textSecondary
                    color: dashPage.fanRelay >= 2 ? themeManager.carBlue : "transparent"
                    Text {
                        anchors.centerIn: parent
                        text: "2"
                        font.pixelSize: 13; font.bold: true
                        color: dashPage.fanRelay >= 2 ? "#ffffff" : themeManager.textSecondary
                    }
                }
            }
        }

        // ── Quick Controls ──────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 90
            color: themeManager.bgCard
            radius: 6
            border.width: 1
            border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                // Climate Section
                Rectangle {
                    Layout.preferredWidth: 260; Layout.fillHeight: true
                    color: themeManager.bgDark; radius: 6
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.18)
                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 6; spacing: 4
                        Text { text: "CLIMATE"; color: themeManager.textSecondary; font.pixelSize: 9 }
                        RowLayout {
                            Layout.fillWidth: true; spacing: 4
                            Button {
                                Layout.preferredWidth: 30; Layout.preferredHeight: 30
                                text: "-"; font.pixelSize: 16; padding: 0
                                highlighted: dashPage.focusIndex === 0
                                onClicked: if (hvacOn && temperature > 60) temperature--
                            }
                            Text {
                                text: temperature.toFixed(0) + "°"
                                color: themeManager.textPrimary; font.pixelSize: 20; font.bold: true
                                Layout.alignment: Qt.AlignCenter
                            }
                            Button {
                                Layout.preferredWidth: 30; Layout.preferredHeight: 30
                                text: "+"; font.pixelSize: 16; padding: 0
                                highlighted: dashPage.focusIndex === 1
                                onClicked: if (hvacOn && temperature < 85) temperature++
                            }
                        }
                    }
                }

                // Fan Speed
                Rectangle {
                    Layout.preferredWidth: 200; Layout.fillHeight: true
                    color: themeManager.bgDark; radius: 6
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.18)
                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 6; spacing: 4
                        Text { text: "FAN"; color: themeManager.textSecondary; font.pixelSize: 9 }
                        RowLayout {
                            spacing: 4; Layout.fillWidth: true
                            Repeater {
                                model: 5
                                Rectangle {
                                    Layout.preferredWidth: 24; Layout.preferredHeight: 24
                                    radius: 12
                                    color: fanSpeed > index ? themeManager.carBlue : themeManager.bgPanel
                                    border.width: dashPage.focusIndex === 2 ? 2 : 1
                                    border.color: dashPage.focusIndex === 2 ? themeManager.carBlue : themeManager.textSecondary
                                    Text {
                                        anchors.centerIn: parent
                                        text: index + 1; color: fanSpeed > index ? "#000" : themeManager.textSecondary
                                        font.pixelSize: 10; font.bold: true
                                    }
                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: { fanSpeed = index + 1; carControlManager.setFanSpeed(fanSpeed) }
                                    }
                                }
                            }
                        }
                    }
                }

                // HVAC toggles
                Rectangle {
                    Layout.preferredWidth: 200; Layout.fillHeight: true
                    color: themeManager.bgDark; radius: 6
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.18)
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 6; spacing: 4
                        Button {
                            Layout.fillWidth: true; Layout.fillHeight: true
                            text: "HVAC"; font.pixelSize: 10; padding: 2
                            highlighted: dashPage.focusIndex === 3
                            palette.button: hvacOn ? themeManager.carBlue : themeManager.bgPanel
                            onClicked: { hvacOn = !hvacOn; carControlManager.setHvacEnabled(hvacOn) }
                        }
                        Button {
                            Layout.fillWidth: true; Layout.fillHeight: true
                            text: "A/C"; font.pixelSize: 10; padding: 2
                            highlighted: dashPage.focusIndex === 4
                            palette.button: acOn ? themeManager.carBlue : themeManager.bgPanel
                            onClicked: { acOn = !acOn; carControlManager.setAcEnabled(acOn); configManager.acEnabled = acOn }
                        }
                        Button {
                            Layout.fillWidth: true; Layout.fillHeight: true
                            text: autoMode ? "AUTO" : "MAN"; font.pixelSize: 10; padding: 2
                            highlighted: dashPage.focusIndex === 5
                            palette.button: autoMode ? themeManager.carBlue : themeManager.bgPanel
                            onClicked: { autoMode = !autoMode; configManager.autoMode = autoMode }
                        }
                    }
                }

                // Vehicle Controls
                Rectangle {
                    Layout.preferredWidth: 260; Layout.fillHeight: true
                    color: themeManager.bgDark; radius: 6
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.18)
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 6; spacing: 4
                        Button {
                            Layout.fillWidth: true; Layout.fillHeight: true
                            text: doorsLocked ? "UNLOCK" : "LOCK"; font.pixelSize: 10; padding: 2
                            highlighted: dashPage.focusIndex === 6
                            palette.button: doorsLocked ? themeManager.carOrange : themeManager.bgPanel
                            onClicked: doorsLocked ? carControlManager.unlockDoors() : carControlManager.lockDoors()
                        }
                        Button {
                            Layout.fillWidth: true; Layout.fillHeight: true
                            text: "↑WIN"; font.pixelSize: 10; padding: 2
                            highlighted: dashPage.focusIndex === 7
                            palette.button: themeManager.bgPanel
                            onClicked: carControlManager.windowsUp()
                        }
                        Button {
                            Layout.fillWidth: true; Layout.fillHeight: true
                            text: "↓WIN"; font.pixelSize: 10; padding: 2
                            highlighted: dashPage.focusIndex === 8
                            palette.button: themeManager.bgPanel
                            onClicked: carControlManager.windowsDown()
                        }
                    }
                }

                // Radio Controls (Kenwood TEA6320T via ESP32 I2C)
                Rectangle {
                    Layout.preferredWidth: 200; Layout.fillHeight: true
                    color: themeManager.bgDark; radius: 6
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.18)
                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 4; spacing: 2
                        Text { text: "RADIO"; color: themeManager.textSecondary; font.pixelSize: 9 }
                        RowLayout {
                            Layout.fillWidth: true; spacing: 4
                            Button {
                                id: radioVolDown
                                Layout.preferredWidth: 28; Layout.preferredHeight: 28
                                text: "-"; font.pixelSize: 16; padding: 0
                                highlighted: dashPage.focusIndex === 9
                                onClicked: carControlManager.setRadioVolume(Math.max(-31, radioVolume - 5))
                            }
                            Text {
                                text: radioMuted ? "M" : radioVolume + " dB"
                                color: radioMuted ? themeManager.statusRed : themeManager.carBlue
                                font.pixelSize: 12; font.bold: true
                                Layout.alignment: Qt.AlignCenter; Layout.minimumWidth: 50
                            }
                            Button {
                                id: radioVolUp
                                Layout.preferredWidth: 28; Layout.preferredHeight: 28
                                text: "+"; font.pixelSize: 16; padding: 0
                                highlighted: dashPage.focusIndex === 10
                                onClicked: carControlManager.setRadioVolume(Math.min(20, radioVolume + 5))
                            }
                            Button {
                                Layout.preferredWidth: 28; Layout.preferredHeight: 28
                                text: "M"; font.pixelSize: 11; padding: 0; font.bold: true
                                highlighted: dashPage.focusIndex === 11
                                palette.button: radioMuted ? themeManager.carOrange : themeManager.bgPanel
                                onClicked: carControlManager.setRadioMuted(!radioMuted)
                            }
                        }
                    }
                }
            }
        }
    }

    // ── Warning Popup Overlay ──────────────────────────────────────────
    Rectangle {
        id: warningPopup
        anchors.fill: parent
        color: "#cc000000"
        visible: dashPage.worstAlert !== "ok" && activeWarnings.length > 0 && _dismissedKey !== dashPage.worstAlert + "_" + activeWarnings.length
        z: 200
        opacity: 0
        Behavior on opacity { NumberAnimation { duration: 300 } }

        property bool dismissed: false

        onVisibleChanged: {
            if (visible) opacity = 1.0
            else opacity = 0.0
        }

        MouseArea {
            anchors.fill: parent
            // Block clicks beneath
        }

        Rectangle {
            anchors.centerIn: parent
            width: Math.min(parent.width - 100, 500)
            height: popupColumn.implicitHeight + 40
            radius: 12
            color: dashPage.worstAlert === "critical" ? "#1a0000" :
                   dashPage.worstAlert === "danger" ? "#1a0f00" : "#00001a"
            border.width: 2
            border.color: dashPage.worstAlert === "critical" ? themeManager.statusRed :
                          dashPage.worstAlert === "danger" ? themeManager.statusOrange :
                          themeManager.statusYellow

            Rectangle {
                anchors.fill: parent
                radius: 12
                color: "transparent"
                border.width: 0
                visible: dashPage.worstAlert === "critical"
                SequentialAnimation on border.color {
                    loops: Animation.Infinite
                    ColorAnimation { from: themeManager.statusRed; to: themeManager.transparent; duration: 500 }
                    ColorAnimation { from: themeManager.transparent; to: themeManager.statusRed; duration: 500 }
                }
            }

            Column {
                id: popupColumn
                anchors { fill: parent; margins: 20 }
                spacing: 10

                Text {
                    text: dashPage.worstAlert === "critical" ? "⚠ CRITICAL WARNING" :
                          dashPage.worstAlert === "danger" ? "⚠ WARNING" : "⚠ CAUTION"
                    color: dashPage.worstAlert === "critical" ? themeManager.statusRed :
                           dashPage.worstAlert === "danger" ? themeManager.statusOrange :
                           themeManager.statusYellow
                    font.pixelSize: 18; font.bold: true
                }

                Rectangle {
                    width: parent.width; height: 1
                    color: dashPage.worstAlert === "critical" ? themeManager.statusRed :
                           dashPage.worstAlert === "danger" ? themeManager.statusOrange :
                           themeManager.statusYellow
                }

                Repeater {
                    model: dashPage.activeWarnings
                    Rectangle {
                        width: parent.width
                        height: 50
                        color: "#22000000"
                        radius: 6
                        Row {
                            anchors { fill: parent; margins: 8 }
                            spacing: 10
                            Rectangle {
                                width: 8; height: 8; radius: 4
                                anchors.verticalCenter: parent.verticalCenter
                                color: modelData.level === "critical" ? themeManager.statusRed :
                                       modelData.level === "danger" ? themeManager.statusOrange :
                                       themeManager.statusYellow
                            }
                            Column {
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 2
                                Text {
                                    text: modelData.sensor + ": " + modelData.value
                                    color: modelData.level === "critical" ? themeManager.statusRed :
                                           modelData.level === "danger" ? themeManager.statusOrange :
                                           themeManager.textPrimary
                                    font.pixelSize: 14; font.bold: true
                                }
                                Text {
                                    text: modelData.action
                                    color: themeManager.textSecondary
                                    font.pixelSize: 11
                                }
                            }
                        }
                    }
                }

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    width: 200; height: 44
                    text: "DISMISS"
                    font.bold: true
                    palette.button: themeManager.bgPanel
                    onClicked: {
                        var key = dashPage.worstAlert + "_" + dashPage.activeWarnings.length
                        _dismissedKey = key
                        warningPopup.dismissed = true
                        warningPopup.opacity = 0
                        warningPopup.visible = false
                    }
                }
            }
        }
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_Escape) {
            if (warningPopup.visible) {
                var key = dashPage.worstAlert + "_" + dashPage.activeWarnings.length
                _dismissedKey = key
                warningPopup.dismissed = true
                warningPopup.opacity = 0
                warningPopup.visible = false
                event.accepted = true; return
            }
        }
        if (event.key === Qt.Key_Left && focusIndex > 0) {
            focusIndex--
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_Right && focusIndex < 16) {
            focusIndex++
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_Up || event.key === Qt.Key_Down) {
            if (focusIndex === 0) { if (hvacOn && temperature > 60) temperature--; event.accepted = true; return }
            if (focusIndex === 1) { if (hvacOn && temperature < 85) temperature++; event.accepted = true; return }
            if (focusIndex === 14) { dashPage.centerView = (dashPage.centerView + 1) % 3; event.accepted = true; return }
        }
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            if (focusIndex === 2) { var s = fanSpeed + 1; if (s > 5) s = 1; fanSpeed = s; carControlManager.setFanSpeed(fanSpeed) }
            else if (focusIndex === 3) { hvacOn = !hvacOn; carControlManager.setHvacEnabled(hvacOn) }
            else if (focusIndex === 4) { acOn = !acOn; carControlManager.setAcEnabled(acOn); configManager.acEnabled = acOn }
            else if (focusIndex === 5) { autoMode = !autoMode; configManager.autoMode = autoMode }
            else if (focusIndex === 6) { if (doorsLocked) carControlManager.unlockDoors(); else carControlManager.lockDoors() }
            else if (focusIndex === 7) { carControlManager.windowsUp() }
            else if (focusIndex === 8) { carControlManager.windowsDown() }
            else if (focusIndex === 9) { carControlManager.setRadioVolume(Math.max(-31, radioVolume - 5)) }
            else if (focusIndex === 10) { carControlManager.setRadioVolume(Math.min(20, radioVolume + 5)) }
            else if (focusIndex === 11) { carControlManager.setRadioMuted(!radioMuted) }
            else if (focusIndex === 12) { mediaManager.previous() }
            else if (focusIndex === 13) { mediaManager.playing ? mediaManager.pause() : mediaManager.play() }
            else if (focusIndex === 14) { mediaManager.next() }
            else if (focusIndex === 15 && tripComputer) { tripComputer.reset() }
            else if (focusIndex === 16) { dashPage.centerView = (dashPage.centerView + 1) % 3 }
            event.accepted = true
            return
        }
    }

    property var _sweepGauges: [speedoGauge, rpmGauge, tempGauge, fuelGauge, tpsGauge, mapGauge, oilPressGauge]
    property int _sweepIdx: 0

    Timer {
        id: sweepTimer
        interval: 120; repeat: true
        onTriggered: {
            if (_sweepIdx < _sweepGauges.length) {
                _sweepGauges[_sweepIdx].runSweep()
                _sweepIdx++
            } else {
                sweepTimer.stop()
                _sweepIdx = 0
            }
        }
    }

    Timer {
        id: startSweep
        interval: 500; repeat: false
        onTriggered: sweepTimer.restart()
    }

    Component.onCompleted: startSweep.start()

    Timer {
        interval: 1000; running: true; repeat: true
        onTriggered: {
            timeText.text = new Date().toLocaleTimeString(Qt.locale("en_US"), "hh:mm AP")
        }
    }

    function activateCurrentFocus() {
        if (focusIndex < 0) { focusIndex = 0; return }
        if (focusIndex === 0) { if (hvacOn && temperature > 60) temperature--; return }
        if (focusIndex === 1) { if (hvacOn && temperature < 85) temperature++; return }
        if (focusIndex === 2) { var s = fanSpeed + 1; if (s > 5) s = 1; fanSpeed = s; carControlManager.setFanSpeed(fanSpeed); return }
        if (focusIndex === 3) { hvacOn = !hvacOn; carControlManager.setHvacEnabled(hvacOn); return }
        if (focusIndex === 4) { acOn = !acOn; carControlManager.setAcEnabled(acOn); configManager.acEnabled = acOn; return }
        if (focusIndex === 5) { autoMode = !autoMode; configManager.autoMode = autoMode; return }
        if (focusIndex === 6) { if (doorsLocked) carControlManager.unlockDoors(); else carControlManager.lockDoors(); return }
        if (focusIndex === 7) { carControlManager.windowsUp(); return }
        if (focusIndex === 8) { carControlManager.windowsDown(); return }
        if (focusIndex === 9) { carControlManager.setRadioVolume(Math.max(-31, radioVolume - 5)); return }
        if (focusIndex === 10) { carControlManager.setRadioVolume(Math.min(20, radioVolume + 5)); return }
        if (focusIndex === 11) { carControlManager.setRadioMuted(!radioMuted); return }
        if (focusIndex === 12) { mediaManager.previous(); return }
        if (focusIndex === 13) { mediaManager.playing ? mediaManager.pause() : mediaManager.play(); return }
        if (focusIndex === 14) { mediaManager.next(); return }
        if (focusIndex === 15 && tripComputer) { tripComputer.reset(); return }
        if (focusIndex === 16) { dashPage.centerView = (dashPage.centerView + 1) % 3; return }
    }

    function navigateLeft()  { if (focusIndex < 0) { focusIndex = 0; return } if (focusIndex > 0) focusIndex-- }
    function navigateRight() { if (focusIndex < 0) { focusIndex = 0; return } if (focusIndex < 16) focusIndex++ }
    function navigateUp() {
        if (focusIndex < 0) { focusIndex = 0; return true }
        if (focusIndex === 0) { if (hvacOn && temperature > 60) temperature--; return true }
        if (focusIndex === 1) { if (hvacOn && temperature < 85) temperature++; return true }
        if (focusIndex === 14) { dashPage.centerView = (dashPage.centerView + 1) % 3; return true }
        return true
    }
    function navigateDown() {
        if (focusIndex < 0) { focusIndex = 0; return true }
        if (focusIndex === 14) { dashPage.centerView = (dashPage.centerView + 1) % 3; return false }
        if (focusIndex === 0 || focusIndex === 1) { focusIndex = 2; return true }
        return false
    }
    function handleEscape() {
        if (warningPopup.visible) {
            var key = dashPage.worstAlert + "_" + dashPage.activeWarnings.length
            _dismissedKey = key
            warningPopup.dismissed = true
            warningPopup.opacity = 0
            warningPopup.visible = false
        }
    }

    function formatTime(ms) {
        if (!ms || ms <= 0) return "0:00"
        var seconds = Math.floor(ms / 1000)
        var minutes = Math.floor(seconds / 60)
        seconds = seconds % 60
        return minutes + ":" + (seconds < 10 ? "0" : "") + seconds
    }

    function formatTripTime(sec) {
        if (!sec || sec <= 0) return "0:00:00"
        var h = Math.floor(sec / 3600)
        var m = Math.floor((sec % 3600) / 60)
        var s = sec % 60
        return h + ":" + (m < 10 ? "0" : "") + m + ":" + (s < 10 ? "0" : "") + s
    }
}
