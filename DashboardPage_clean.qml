import QtQuick 2.15
import QtQuick.Controls 2.15

Item {
    focus: true

    property int fanSpeed: carControlManager ? carControlManager.fanSpeed : 0
    property bool hvacOn: configManager.hvacEnabled
    property bool acOn: configManager.acEnabled
    property bool doorsLocked: carControlManager ? carControlManager.doorsLocked : false
    property bool remoteStartActive: carControlManager ? carControlManager.remoteStartActive : false
    property real temperature: configManager.targetTemp

    // From ClimatePage
    property bool autoMode: configManager.autoMode
    property bool recirculate: configManager.recirculate

    // ── Engine Profile Derived Values ────────────────────────────────────
    property int _redlineRPM: engineProfile ? engineProfile.redlineRPM : 6500
    property int _fuelCutRPM: engineProfile ? engineProfile.fuelCutRPM : 7500
    property int _rpmGaugeMax: engineProfile ? engineProfile.rpmGaugeMax : 8000
    property double _rpmWarnFraction: _rpmGaugeMax > 0 ? Math.min((_redlineRPM - 500) / _rpmGaugeMax, 0.85) : 0.75
    property double _rpmDangerFraction: _rpmGaugeMax > 0 ? _fuelCutRPM / _rpmGaugeMax : 0.9

    property int _coolantGaugeMin: engineProfile ? engineProfile.coolantGaugeMin : 100
    property int _coolantGaugeMax: engineProfile ? engineProfile.coolantGaugeMax : 300
    property double _coolantWarnFraction: _coolantGaugeMax > _coolantGaugeMin ? (engineProfile.coolantCautionF - _coolantGaugeMin) / (_coolantGaugeMax - _coolantGaugeMin) : 0.55
    property double _coolantDangerFraction: _coolantGaugeMax > _coolantGaugeMin ? (engineProfile.coolantDangerF - _coolantGaugeMin) / (_coolantGaugeMax - _coolantGaugeMin) : 0.7

    property int _speedGaugeMax: engineProfile ? engineProfile.speedGaugeMaxMPH : 120
    property double _speedWarnFraction: engineProfile ? Math.min(engineProfile.speedWarnMPH / _speedGaugeMax, 0.9) : 0.8
    property double _speedDangerFraction: engineProfile ? Math.min(engineProfile.speedDangerMPH / _speedGaugeMax, 0.95) : 0.9

    Connections {
        target: carControlManager
        function onFanSpeedChanged() { fanSpeed = carControlManager.fanSpeed }
        function onHvacEnabledChanged() { hvacOn = carControlManager.hvacEnabled }
        function onAcEnabledChanged() { acOn = carControlManager.acEnabled }
        function onDoorsLockedChanged() { doorsLocked = carControlManager.doorsLocked }
        function onRemoteStartActiveChanged() { remoteStartActive = carControlManager.remoteStartActive }
    }

    // Save to config when UI changes
    onTemperatureChanged: configManager.targetTemp = temperature

    Rectangle {
        anchors.fill: parent
        color: root.bgDark

        Flickable {
            anchors.fill: parent
            contentHeight: mainColumn.implicitHeight + 40
            clip: true

            Column {
                id: mainColumn
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 10
                spacing: 8

                // ── Top row - time, system, quick actions ───────────────
                Rectangle {
                    width: parent.width - 20
                    height: 80
                    color: root.bgCard
                    radius: 8

                    Row {
                        anchors.fill: parent
                        padding: 12
                        spacing: 15

                        Column {
                            width: parent.width / 3 - 20
                            Text {
                                text: "TIME"
                                color: root.textSecondary
                                font.pixelSize: 11
                            }
                            Text {
                                id: timeText
                                text: new Date().toLocaleTimeString(Qt.locale("en_US"), "h:mm:ss AP")
                                color: root.textPrimary
                                font.pixelSize: 28
                                font.bold: true
                            }
                            Text {
                                text: new Date().toLocaleDateString(Qt.locale("en_US"), "dddd, MMMM d")
                                color: root.textSecondary
                                font.pixelSize: 12
                            }
                        }

                        Column {
                            width: parent.width / 3 - 20
                            Text {
                                text: "SYSTEM"
                                color: root.textSecondary
                                font.pixelSize: 11
                            }
                            Text {
                                text: "v" + appVersion
                                color: root.carBlue
                                font.pixelSize: 20
                                font.bold: true
                            }
                            Text {
                                text: carControlManager && carControlManager.connected ? "All Systems OK" : "Check ESP32"
                                color: carControlManager && carControlManager.connected ? "#00ff00" : "#ff0000"
                                font.pixelSize: 12
                            }
                        }

                        Column {
                            width: parent.width / 3 - 20
                            Text {
                                text: "QUICK ACTIONS"
                                color: root.textSecondary
                                font.pixelSize: 11
                            }
                            Row {
                                spacing: 8
                                Button {
                                    width: 55; height: 35
                                    text: "Media"
                                    font.pixelSize: 11
                                    onClicked: root.activePage = 2
                                }
                                Button {
                                    width: 55; height: 35
                                    text: "Rear"
                                    font.pixelSize: 11
                                    onClicked: root.activePage = 3
                                }
                                Button {
                                    width: 55; height: 35
                                    text: "DVR"
                                    font.pixelSize: 11
                                    onClicked: root.activePage = 4
                                }
                            }
                        }
                    }
                }

                }

                // ── Gauges row ────────────────────────────────────────
                Row {
                    spacing: 8
                    width: parent.width - 20
                    height: 200

                    // Speedometer
                    AnalogGauge {
                        width: (parent.width - 24) / 4
                        height: parent.height
                        value: sensorManager ? sensorManager.speed : 0
                        minValue: 0
                        maxValue: _speedGaugeMax
                        label: "MPH"
                        fontSize: 22
                        gaugeColor: root.carBlue
                        bgColor: root.bgCard
                        tickColor: "#666666"
                        textColor: root.textSecondary
                        startAngle: 135
                        endAngle: 405
                        warnValue: _speedWarnFraction
                        dangerValue: _speedDangerFraction
                    }

                    // RPM Gauge
                    AnalogGauge {
                        width: (parent.width - 24) / 4
                        height: parent.height
                        value: sensorManager ? sensorManager.rpm : 0
                        minValue: 0
                        maxValue: _rpmGaugeMax
                        label: "RPM"
                        fontSize: 18
                        gaugeColor: (sensorManager && sensorManager.rpm > _redlineRPM) ? "#ff4444" : root.carOrange
                        bgColor: root.bgCard
                        tickColor: "#666666"
                        textColor: root.textSecondary
                        startAngle: 135
                        endAngle: 405
                        warnValue: _rpmWarnFraction
                        dangerValue: _rpmDangerFraction
                    }

                    // Fuel Gauge
                    AnalogGauge {
                        width: (parent.width - 24) / 4
                        height: parent.height
                        value: sensorManager ? sensorManager.fuelLevel : 0
                        minValue: 0
                        maxValue: 100
                        label: "FUEL"
                        fontSize: 16
                        gaugeColor: (sensorManager && sensorManager.fuelLevel > 25) ? "#00ff00" : "#ff4444"
                        bgColor: root.bgCard
                        tickColor: "#666666"
                        textColor: root.textSecondary
                        startAngle: 135
                        endAngle: 405
                        warnValue: 0.25
                        dangerValue: 0.1
                    }

                    // Coolant Temp Gauge
                    AnalogGauge {
                        width: (parent.width - 24) / 4
                        height: parent.height
                        value: sensorManager ? sensorManager.coolantTemp : 0
                        minValue: _coolantGaugeMin
                        maxValue: _coolantGaugeMax
                        label: "TEMP"
                        fontSize: 16
                        gaugeColor: (sensorManager && sensorManager.coolantTemp > _coolantGaugeMin + (_coolantGaugeMax - _coolantGaugeMin) * _coolantWarnFraction) ? "#ff4444" : "#ff6600"
                        bgColor: root.bgCard
                        tickColor: "#666666"
                        textColor: root.textSecondary
                        startAngle: 135
                        endAngle: 405
                        warnValue: _coolantWarnFraction
                        dangerValue: _coolantDangerFraction
                    }
                }

                // ── Bottom row - 4 cards (Vehicle, Fuel, Controller) ───
                Row {
                    spacing: 8
                    width: parent.width - 20
                    height: 75

                    // Vehicle Card
                    Rectangle {
                        width: Math.floor((parent.width - 24) / 4)
                        height: parent.height
                        color: root.bgCard
                        radius: 8

                        Column {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 4
                            Text {
                                text: "VEHICLE"
                                color: root.textSecondary
                                font.pixelSize: 10
                            }
                            Text {
                                text: doorsLocked ? "LOCKED" : "UNLOCKED"
                                color: doorsLocked ? root.carOrange : "#00ff00"
                                font.pixelSize: 18
                                font.bold: true
                            }
                            Text {
                                text: remoteStartActive ? "Engine On" : "Engine Off"
                                color: remoteStartActive ? root.carOrange : root.textSecondary
                                font.pixelSize: 9
                            }
                        }
                    }

                    // Fuel Card
                    Rectangle {
                        width: Math.floor((parent.width - 24) / 4)
                        height: parent.height
                        color: root.bgCard
                        radius: 8

                        Column {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 4
                            Text {
                                text: "FUEL"
                                color: root.textSecondary
                                font.pixelSize: 10
                            }
                            Text {
                                text: sensorManager ? sensorManager.fuelLevel + "%" : "--%"
                                color: sensorManager && sensorManager.fuelLevel > 25 ? "#00ff00" : "#ff4444"
                                font.pixelSize: 20
                                font.bold: true
                            }
                            Text {
                                text: sensorManager && sensorManager.fuelLevel > 0 ? "Level OK" : "No data"
                                color: root.textSecondary
                                font.pixelSize: 9
                            }
                        }
                    }

                    // Controller Card
                    Rectangle {
                        width: Math.floor((parent.width - 24) / 4)
                        height: parent.height
                        color: root.bgCard
                        radius: 8

                        Column {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 4
                            Text {
                                text: "CONTROLLER"
                                color: root.textSecondary
                                font.pixelSize: 10
                            }
                            Text {
                                text: carControlManager && carControlManager.connected ? "CONNECTED" : "OFFLINE"
                                color: carControlManager && carControlManager.connected ? "#00ff00" : "#ff0000"
                                font.pixelSize: 18
                                font.bold: true
                            }
                            Text {
                                text: carControlManager && carControlManager.connected ? "ESP32 OK" : "Check Connection"
                                color: root.textSecondary
                                font.pixelSize: 9
                            }
                        }
                    }
                }

                // ── Divider ────────────────────────────────────────────
                Rectangle {
                    width: parent.width - 20
                    height: 1
                    color: root.bgPanel
                }

                // ── FULL CLIMATE CONTROLS (from ClimatePage) ───────────
                Text {
                    text: "CLIMATE CONTROLS"
                    color: root.carBlue
                    font.pixelSize: 18
                    font.bold: true
                }

                // HVAC Power Toggle
                Row {
                    spacing: 20
                    anchors.horizontalCenter: parent.horizontalCenter

                    Button {
                        width: 180
                        height: 50
                        text: hvacOn ? "HVAC ON" : "HVAC OFF"
                        palette.button: hvacOn ? root.carBlue : root.bgCard
                        font.bold: true
                        onClicked: {
                            hvacOn = !hvacOn
                            carControlManager.setHvacEnabled(hvacOn)
                        }
                    }
                }

                // Temperature control
                Rectangle {
                    width: parent.width - 20
                    height: 100
                    color: root.bgCard
                    radius: 8

                    Row {
                        anchors.fill: parent
                        padding: 20
                        spacing: 40

                        Column {
                            Text {
                                text: "CURRENT"
                                color: root.textSecondary
                                font.pixelSize: 14
                            }
                            Text {
                                text: temperature.toFixed(0) + "°F"
                                color: root.textPrimary
                                font.pixelSize: 48
                                font.bold: true
                            }
                        }

                        Column {
                            Text {
                                text: "TARGET"
                                color: root.textSecondary
                                font.pixelSize: 14
                            }
                            Row {
                                spacing: 10
                                Button {
                                    width: 40; height: 40
                                    text: "-"
                                    font.pixelSize: 12
                                    padding: 0
                                    onClicked: {
                                        if (hvacOn && temperature > 60) {
                                            temperature--
                                            configManager.targetTemp = temperature
                                        }
                                    }
                                }
                                Button {
                                    width: 40; height: 40
                                    text: "+"
                                    font.pixelSize: 12
                                    padding: 0
                                    onClicked: {
                                        if (hvacOn && temperature < 85) {
                                            temperature++
                                            configManager.targetTemp = temperature
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Text {
                    text: "FAN SPEED"
                    color: root.textSecondary
                    font.pixelSize: 14
                }

                Row {
                    spacing: 10
                    anchors.horizontalCenter: parent.horizontalCenter

                    Repeater {
                        model: 5
                        delegate: Rectangle {
                            width: 50; height: 50
                            color: fanSpeed > index ? root.carBlue : root.bgCard
                            radius: 8
                            Text {
                                anchors.centerIn: parent
                                text: index + 1
                                color: fanSpeed > index ? "#000000" : root.textPrimary
                                font.pixelSize: 20
                                font.bold: true
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    fanSpeed = index + 1
                                    carControlManager.setFanSpeed(fanSpeed)
                                }
                            }
                        }
                    }
                }

                Row {
                    spacing: 20
                    anchors.horizontalCenter: parent.horizontalCenter

                    Button {
                        width: 120; height: 60
                        text: autoMode ? "AUTO" : "MANUAL"
                        palette.button: autoMode ? root.carBlue : root.bgCard
                        onClicked: {
                            autoMode = !autoMode
                            configManager.autoMode = autoMode
                        }
                    }
                    Button {
                        width: 120; height: 60
                        text: acOn ? "A/C ON" : "A/C OFF"
                        palette.button: acOn ? root.carBlue : root.bgCard
                        onClicked: {
                            acOn = !acOn
                            carControlManager.setAcEnabled(acOn)
                            configManager.acEnabled = acOn
                        }
                    }
                    Button {
                        width: 120; height: 60
                        text: recirculate ? "RECIRC" : "FRESH"
                        palette.button: recirculate ? root.carBlue : root.bgCard
                        onClicked: {
                            recirculate = !recirculate
                            configManager.recirculate = recirculate
                        }
                    }
                }

                Text {
                    text: "AIRFLOW MODE"
                    color: root.textSecondary
                    font.pixelSize: 14
                }

                Row {
                    spacing: 10
                    anchors.horizontalCenter: parent.horizontalCenter

                    Button { width: 80; height: 50; text: "FACE" }
                    Button { width: 80; height: 50; text: "FEET" }
                    Button { width: 80; height: 50; text: "DEFROST" }
                    Button { width: 80; height: 50; text: "FEET+DEF" }
                }

                Rectangle {
                    width: parent.width - 20
                    height: 1
                    color: root.bgPanel
                }

                Text {
                    text: "VEHICLE CONTROLS"
                    color: root.textSecondary
                    font.pixelSize: 14
                }

                Row {
                    spacing: 20
                    anchors.horizontalCenter: parent.horizontalCenter

                    Button {
                        width: 120; height: 60
                        text: doorsLocked ? "UNLOCK" : "LOCK"
                        palette.button: doorsLocked ? root.carOrange : root.bgCard
                        onClicked: {
                            if (doorsLocked) {
                                carControlManager.unlockDoors()
                            } else {
                                carControlManager.lockDoors()
                            }
                        }
                    }
                    Button {
                        width: 120; height: 60
                        text: "WIN UP"
                        palette.button: root.bgCard
                        onClicked: carControlManager.windowsUp()
                    }
                    Button {
                        width: 120; height: 60
                        text: "WIN DOWN"
                        palette.button: root.bgCard
                        onClicked: carControlManager.windowsDown()
                    }
                }

                Row {
                    spacing: 20
                    anchors.horizontalCenter: parent.horizontalCenter

                    Button {
                        width: 180; height: 70
                        text: remoteStartActive ? "STOP ENGINE" : "REMOTE START"
                        palette.button: remoteStartActive ? root.carOrange : root.carBlue
                        font.pixelSize: 16
                        font.bold: true
                        onClicked: {
                            if (remoteStartActive) {
                                carControlManager.stopRemote()
                            } else {
                                carControlManager.startRemote()
                            }
                        }
                    }
                }

                Rectangle {
                    width: parent.width - 20
                    height: 1
                    color: root.bgPanel
                }
            }
        }
    }

    Timer {
        interval: 1000
        running: true
        repeat: true
        onTriggered: {
            timeText.text = new Date().toLocaleTimeString(Qt.locale("en_US"), "h:mm:ss AP")
        }
    }

    function formatTime(ms) {
        if (!ms || ms <= 0) return "0:00"
        var seconds = Math.floor(ms / 1000)
        var minutes = Math.floor(seconds / 60)
        seconds = seconds % 60
        return minutes + ":" + (seconds < 10 ? "0" : "") + seconds
    }
}
