import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: btPage
    anchors.fill: parent
    focus: true

    property var navModel: ["<-back"]
    property int selectedNav: 0

    Keys.onPressed: {
        if (event.key === Qt.Key_Left || event.key === Qt.Key_Escape) {
            root.activePage = 1
            event.accepted = true
        } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            if (selectedNav === 0) root.activePage = 1
            event.accepted = true
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // ── Header ───────────────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true
            Text {
                text: "BLUETOOTH / OBD2"
                font.pixelSize: 24
                font.bold: true
                color: themeManager.textPrimary
            }
            Item { Layout.fillWidth: true }
            Rectangle {
                width: 12; height: 12; radius: 6
                color: bluetoothManager.connected ? "#4CAF50" : "#F44336"
                SequentialAnimation on opacity {
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.3; duration: 800 }
                    NumberAnimation { to: 1.0; duration: 800 }
                    running: !bluetoothManager.connected
                }
            }
            Text {
                text: bluetoothManager.connected ? "CONNECTED" : "DISCONNECTED"
                font.pixelSize: 14
                font.bold: true
                color: bluetoothManager.connected ? "#4CAF50" : "#F44336"
            }
        }

        // ── Status ───────────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            height: 32
            color: Qt.rgba(1, 1, 1, 0.05)
            radius: 4
            Text {
                anchors.centerIn: parent
                text: bluetoothManager.statusText
                font.pixelSize: 12
                color: Qt.rgba(themeManager.textPrimary.r, themeManager.textPrimary.g, themeManager.textPrimary.b, 0.7)
            }
        }

        // ── Actions row ──────────────────────────────────────────
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                text: "SCAN"
                Layout.fillWidth: true
                onClicked: bluetoothManager.scanDevices()
                enabled: !bluetoothManager.connected
            }
            Button {
                text: "STATUS"
                Layout.fillWidth: true
                onClicked: bluetoothManager.refreshStatus()
            }
            Button {
                text: "DISCONNECT"
                Layout.fillWidth: true
                onClicked: bluetoothManager.disconnectDevice()
                enabled: bluetoothManager.connected
            }
        }

        // ── Device name ──────────────────────────────────────────
        Rectangle {
            Layout.fillWidth: true
            height: 36
            color: Qt.rgba(1, 1, 1, 0.05)
            radius: 4
            visible: bluetoothManager.deviceName.length > 0
            RowLayout {
                anchors.centerIn: parent
                spacing: 8
                Text {
                    text: "Paired:"
                    font.pixelSize: 13
                    font.bold: true
                    color: themeManager.textPrimary
                }
                Text {
                    text: bluetoothManager.deviceName
                    font.pixelSize: 13
                    color: "#4CAF50"
                }
                Text {
                    text: "(" + bluetoothManager.deviceAddress + ")"
                    font.pixelSize: 11
                    color: Qt.rgba(themeManager.textPrimary.r, themeManager.textPrimary.g, themeManager.textPrimary.b, 0.5)
                }
            }
        }

        // ── OBD2 Live Data ───────────────────────────────────────
        Text {
            text: "OBD2 LIVE DATA"
            font.pixelSize: 16
            font.bold: true
            color: themeManager.textPrimary
        }

        GridLayout {
            Layout.fillWidth: true
            columns: 4
            columnSpacing: 8
            rowSpacing: 4

            // Row 1: RPM, Speed, Load, Timing
            Repeater {
                model: [
                    { label: "RPM", value: bluetoothManager.obd2Rpm, unit: "" },
                    { label: "SPEED", value: bluetoothManager.obd2Speed, unit: "km/h" },
                    { label: "LOAD", value: bluetoothManager.obd2Load, unit: "%" },
                    { label: "TIMING", value: bluetoothManager.obd2Timing, unit: "°" }
                ]
                delegate: Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 160
                    height: 52
                    color: Qt.rgba(1, 1, 1, 0.05)
                    radius: 4
                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 2
                        Text {
                            text: modelData.label
                            font.pixelSize: 10
                            font.bold: true
                            color: Qt.rgba(themeManager.textPrimary.r, themeManager.textPrimary.g, themeManager.textPrimary.b, 0.6)
                            Layout.alignment: Qt.AlignHCenter
                        }
                        Text {
                            text: modelData.value + modelData.unit
                            font.pixelSize: 20
                            font.bold: true
                            color: themeManager.textPrimary
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
            }

            // Row 2: Coolant, Intake, Fuel Trim S, Fuel Trim L
            Repeater {
                model: [
                    { label: "COOLANT", value: bluetoothManager.obd2Coolant, unit: "°F" },
                    { label: "INTAKE", value: bluetoothManager.obd2Intake, unit: "°F" },
                    { label: "STFT", value: bluetoothManager.obd2FuelTrimShort, unit: "%" },
                    { label: "LTFT", value: bluetoothManager.obd2FuelTrimLong, unit: "%" }
                ]
                delegate: Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 160
                    height: 52
                    color: Qt.rgba(1, 1, 1, 0.05)
                    radius: 4
                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 2
                        Text {
                            text: modelData.label
                            font.pixelSize: 10
                            font.bold: true
                            color: Qt.rgba(themeManager.textPrimary.r, themeManager.textPrimary.g, themeManager.textPrimary.b, 0.6)
                            Layout.alignment: Qt.AlignHCenter
                        }
                        Text {
                            text: modelData.value + modelData.unit
                            font.pixelSize: 20
                            font.bold: true
                            color: themeManager.textPrimary
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
            }

            // Row 3: Fuel Pressure, Distance DTC
            Repeater {
                model: [
                    { label: "FUEL PRESS", value: bluetoothManager.obd2FuelPressure, unit: "kPa" },
                    { label: "DTC DIST", value: bluetoothManager.obd2DistanceDtc, unit: "km" }
                ]
                delegate: Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 160
                    height: 52
                    color: Qt.rgba(1, 1, 1, 0.05)
                    radius: 4
                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 2
                        Text {
                            text: modelData.label
                            font.pixelSize: 10
                            font.bold: true
                            color: Qt.rgba(themeManager.textPrimary.r, themeManager.textPrimary.g, themeManager.textPrimary.b, 0.6)
                            Layout.alignment: Qt.AlignHCenter
                        }
                        Text {
                            text: modelData.value + modelData.unit
                            font.pixelSize: 20
                            font.bold: true
                            color: themeManager.textPrimary
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }

        // ── Bottom nav ───────────────────────────────────────────
        Text {
            text: "< BACK"
            font.pixelSize: 14
            color: themeManager.accentColor
            MouseArea {
                anchors.fill: parent
                onClicked: root.activePage = 1
            }
        }
    }

    Component.onCompleted: {
        bluetoothManager.refreshStatus()
    }
}
