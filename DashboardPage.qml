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
    property real temperature: configManager.targetTemp
    property bool autoMode: configManager.autoMode
    property bool recirculate: configManager.recirculate
    property int centerView: 0

    // ── Warning system ──────────────────────────────────────────────────
    property int _lastAlertLevel: 0
    property string coolantAlert: {
        if (!sensorManager) return "ok"
        var t = sensorManager.coolantTemp
        if (t > 245) return "critical"
        if (t > 230) return "danger"
        if (t > 220) return "caution"
        return "ok"
    }
    property string oilTempAlert: {
        if (!sensorManager) return "ok"
        var t = sensorManager.oilTemp
        if (t > 260) return "critical"
        if (t > 240) return "danger"
        if (t > 220) return "caution"
        return "ok"
    }
    property string oilPressureAlert: {
        if (!sensorManager) return "ok"
        var p = sensorManager.oilPressure
        if (p <= 0) return "ok"
        if (p < 10) return "critical"
        if (p < 20) return "danger"
        if (p < 40) return "caution"
        return "ok"
    }
    property string batteryAlert: {
        if (!sensorManager) return "ok"
        var b = sensorManager.battery
        if (b <= 0) return "ok"
        if (b < 10) return "critical"
        if (b < 25) return "danger"
        if (b < 50) return "caution"
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
    }
    onTemperatureChanged: configManager.targetTemp = temperature

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6

        // ── Status Bar ─────────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            color: themeManager.bgCard
            radius: 6
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
                    // Alert level badge
                    Rectangle {
                        width: 8; height: 8; radius: 4
                        color: dashPage.worstAlert === "critical" ? themeManager.statusRed :
                              dashPage.worstAlert === "danger" ? themeManager.statusOrange :
                              dashPage.worstAlert === "caution" ? themeManager.statusYellow :
                              themeManager.statusGreen
                        visible: sensorManager && sensorManager.connected
                    }
                    Text {
                        text: dashPage.worstAlert === "ok" ? "OK" : dashPage.worstAlert.toUpperCase()
                        color: dashPage.worstAlert === "ok" ? themeManager.statusGreen : themeManager.textPrimary
                        font.pixelSize: 10; font.bold: dashPage.worstAlert !== "ok"
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
                anchors { left: parent.left; leftMargin: 20; verticalCenter: parent.verticalCenter }
                value: sensorManager ? sensorManager.speed : 0
                minValue: 0; maxValue: 120
                label: "SPEED"
                unitLabel: "mph"
                fontSize: 22
                majorTicks: 6
                minorTicks: 4
                warnValue: 0.75; dangerValue: 0.92
                redlineStart: 0.92
                thickness: 0.12
            }

            // RPM Gauge
            AnalogGauge {
                id: rpmGauge
                width: 380; height: 380
                anchors { right: parent.right; rightMargin: 20; verticalCenter: parent.verticalCenter }
                value: sensorManager ? sensorManager.rpm : 0
                minValue: 0; maxValue: 8000
                label: "ENGINE"
                unitLabel: "rpm"
                fontSize: 22
                majorTicks: 8
                minorTicks: 4
                warnValue: 0.75; dangerValue: 0.875
                redlineStart: 0.75
                thickness: 0.12
            }

            // Central Info Panel
            Rectangle {
                anchors {
                    left: speedoGauge.right; leftMargin: 12
                    right: rpmGauge.left; rightMargin: 12
                    verticalCenter: parent.verticalCenter
                }
                height: 420
                color: themeManager.bgCard
                radius: 10
                clip: true

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
                            Button { text: "◀◀"; width: 50; onClicked: mediaManager.previous() }
                            Button {
                                text: mediaManager.playing ? "⏸" : "▶"; width: 70
                                onClicked: mediaManager.playing ? mediaManager.pause() : mediaManager.play()
                            }
                            Button { text: "▶▶"; width: 50; onClicked: mediaManager.next() }
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
                        spacing: 8
                        Text {
                            text: "TRIP COMPUTER"
                            color: themeManager.textSecondary
                            font.pixelSize: 11; font.bold: true
                        }

                        Row {
                            spacing: 20; anchors.horizontalCenter: parent.horizontalCenter
                            Column { spacing: 4
                                Text { text: "SPEED"; color: themeManager.textSecondary; font.pixelSize: 10 }
                                Text { text: sensorManager ? sensorManager.speed + " mph" : "--"; color: themeManager.carBlue; font.pixelSize: 32; font.bold: true }
                            }
                            Column { spacing: 4
                                Text { text: "RPM"; color: themeManager.textSecondary; font.pixelSize: 10 }
                                Text { text: sensorManager ? sensorManager.rpm : "--"; color: themeManager.carOrange; font.pixelSize: 32; font.bold: true }
                            }
                        }

                        Rectangle { width: parent.width; height: 1; color: themeManager.bgPanel }

                        Grid {
                            columns: 2; spacing: 12; width: parent.width
                            Column { spacing: 2; width: (parent.width - 12) / 2
                                Text { text: "FUEL"; color: themeManager.textSecondary; font.pixelSize: 10 }
                                Text { text: sensorManager ? sensorManager.fuelLevel + "%" : "--"; color: themeManager.textPrimary; font.pixelSize: 18; font.bold: true }
                            }
                            Column { spacing: 2; width: (parent.width - 12) / 2
                                Text { text: "COOLANT"; color: themeManager.textSecondary; font.pixelSize: 10 }
                                Text { text: sensorManager ? sensorManager.coolantTemp + "°F" : "--"; color: themeManager.textPrimary; font.pixelSize: 18; font.bold: true }
                            }
                            Column { spacing: 2; width: (parent.width - 12) / 2
                                Text { text: "BATTERY"; color: themeManager.textSecondary; font.pixelSize: 10 }
                                Text { text: sensorManager ? sensorManager.battery + "%" : "--"; color: themeManager.textPrimary; font.pixelSize: 18; font.bold: true }
                            }
                            Column { spacing: 2; width: (parent.width - 12) / 2
                                Text { text: "OIL PSI"; color: themeManager.textSecondary; font.pixelSize: 10 }
                                Text { text: sensorManager ? sensorManager.oilPressure : "--"; color: themeManager.textPrimary; font.pixelSize: 18; font.bold: true }
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
                anchors { left: speedoGauge.left; top: speedoGauge.bottom; topMargin: 6 }
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
                id: tempGauge
                width: 160; height: 120
                anchors { right: rpmGauge.right; top: rpmGauge.bottom; topMargin: 6 }
                value: sensorManager ? sensorManager.coolantTemp : 0
                minValue: 100; maxValue: 280
                label: "COOLANT"
                unitLabel: "°F"
                fontSize: 11
                majorTicks: 4
                minorTicks: 2
                showValue: true; showNeedle: true
                startAngle: 180; endAngle: 360
                warnValue: 0.67; dangerValue: 0.78
                redlineStart: 0.67
                thickness: 0.22
            }

            // Fan status indicator
            Row {
                anchors { horizontalCenter: tempGauge.horizontalCenter; bottom: tempGauge.top; bottomMargin: 4 }
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

                // DEBUG: Fan relay value indicator
                Rectangle {
                    width: 60; height: 28; radius: 4
                    color: "#aa000000"
                    border.width: 2
                    border.color: dashPage.fanRelay > 0 ? themeManager.carBlue : themeManager.textSecondary
                    Text {
                        anchors.centerIn: parent
                        text: "F:" + dashPage.fanRelay
                        font.pixelSize: 14; font.bold: true
                        color: dashPage.fanRelay > 0 ? themeManager.carBlue : themeManager.textSecondary
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
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                // Climate Section
                Rectangle {
                    Layout.preferredWidth: 260; Layout.fillHeight: true
                    color: themeManager.bgDark; radius: 6
                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 6; spacing: 4
                        Text { text: "CLIMATE"; color: themeManager.textSecondary; font.pixelSize: 9 }
                        RowLayout {
                            Layout.fillWidth: true; spacing: 4
                            Button {
                                Layout.preferredWidth: 30; Layout.preferredHeight: 30
                                text: "-"; font.pixelSize: 16; padding: 0
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
                                onClicked: if (hvacOn && temperature < 85) temperature++
                            }
                        }
                    }
                }

                // Fan Speed
                Rectangle {
                    Layout.preferredWidth: 200; Layout.fillHeight: true
                    color: themeManager.bgDark; radius: 6
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
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 6; spacing: 4
                        Button {
                            Layout.fillWidth: true; Layout.fillHeight: true
                            text: "HVAC"; font.pixelSize: 10; padding: 2
                            palette.button: hvacOn ? themeManager.carBlue : themeManager.bgPanel
                            onClicked: { hvacOn = !hvacOn; carControlManager.setHvacEnabled(hvacOn) }
                        }
                        Button {
                            Layout.fillWidth: true; Layout.fillHeight: true
                            text: "A/C"; font.pixelSize: 10; padding: 2
                            palette.button: acOn ? themeManager.carBlue : themeManager.bgPanel
                            onClicked: { acOn = !acOn; carControlManager.setAcEnabled(acOn); configManager.acEnabled = acOn }
                        }
                        Button {
                            Layout.fillWidth: true; Layout.fillHeight: true
                            text: autoMode ? "AUTO" : "MAN"; font.pixelSize: 10; padding: 2
                            palette.button: autoMode ? themeManager.carBlue : themeManager.bgPanel
                            onClicked: { autoMode = !autoMode; configManager.autoMode = autoMode }
                        }
                    }
                }

                // Vehicle Controls
                Rectangle {
                    Layout.preferredWidth: 260; Layout.fillHeight: true
                    color: themeManager.bgDark; radius: 6
                    RowLayout {
                        anchors.fill: parent; anchors.margins: 6; spacing: 4
                        Button {
                            Layout.fillWidth: true; Layout.fillHeight: true
                            text: doorsLocked ? "UNLOCK" : "LOCK"; font.pixelSize: 10; padding: 2
                            palette.button: doorsLocked ? themeManager.carOrange : themeManager.bgPanel
                            onClicked: doorsLocked ? carControlManager.unlockDoors() : carControlManager.lockDoors()
                        }
                        Button {
                            Layout.fillWidth: true; Layout.fillHeight: true
                            text: "↑WIN"; font.pixelSize: 10; padding: 2
                            palette.button: themeManager.bgPanel
                            onClicked: carControlManager.windowsUp()
                        }
                        Button {
                            Layout.fillWidth: true; Layout.fillHeight: true
                            text: "↓WIN"; font.pixelSize: 10; padding: 2
                            palette.button: themeManager.bgPanel
                            onClicked: carControlManager.windowsDown()
                        }
                    }
                }

                // Remote Start
                Rectangle {
                    Layout.preferredWidth: 130; Layout.fillHeight: true
                    color: remoteStartActive ? "#331100" : themeManager.bgDark
                    radius: 6; border.color: remoteStartActive ? themeManager.carOrange : "transparent"
                    border.width: remoteStartActive ? 1 : 0
                    ColumnLayout {
                        anchors.centerIn: parent; spacing: 2
                        Text {
                            text: remoteStartActive ? "● RUNNING" : "REMOTE"
                            color: remoteStartActive ? themeManager.carOrange : themeManager.textSecondary
                            font.pixelSize: 10; font.bold: true
                        }
                        Button {
                            text: remoteStartActive ? "STOP" : "START"
                            font.pixelSize: 11; font.bold: true
                            palette.button: remoteStartActive ? themeManager.statusRed : themeManager.carBlue
                            Layout.alignment: Qt.AlignCenter
                            onClicked: {
                                if (remoteStartActive) carControlManager.stopRemote()
                                else carControlManager.startRemote()
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

    Timer {
        interval: 1000; running: true; repeat: true
        onTriggered: {
            timeText.text = new Date().toLocaleTimeString(Qt.locale("en_US"), "hh:mm AP")
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
