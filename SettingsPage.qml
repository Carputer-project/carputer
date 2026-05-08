import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
Item {
    anchors.fill: parent
    Rectangle {
        anchors.fill: parent
        color: themeManager.bgDark
        Flickable {
            anchors.fill: parent
            contentHeight: mainColumn.implicitHeight + 40
            clip: true
            ColumnLayout {
                id: mainColumn
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 20
                spacing: 15
                Text {
                    text: "SETTINGS"
                    color: themeManager.carBlue
                    font.pixelSize: 24
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }
                // Theme Section
                Rectangle {
                    Layout.fillWidth: true
                    height: themeColumn.implicitHeight + 20
                    color: themeManager.bgCard
                    radius: 8
                    Column {
                        id: themeColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 10
                        spacing: 8
                        Text {
                            text: "Theme"
                            color: themeManager.carBlue
                            font.pixelSize: 16
                            font.bold: true
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10
                            Repeater {
                                model: ["Dark", "Light", "Blue", "Red", "Green", "Purple", "Orange"]
                                Button {
                                    text: modelData
                                    Layout.fillWidth: true
                                    background: Rectangle {
                                        color: themeManager.currentTheme === modelData ? themeManager.carBlueDim : themeManager.bgPanel
                                        radius: 6
                                        border.color: themeManager.carBlueDim
                                        border.width: 1
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        color: themeManager.textPrimary
                                        font.pixelSize: 14
                                        horizontalAlignment: Text.AlignHCenter
                                    }
                                    onClicked: {
                                        var newTheme = modelData
                                        console.log("Switching to theme: " + newTheme)
                                        themeManager.setCurrentTheme(newTheme)
                                        configManager.setTheme(newTheme)
                                    }
                                }
                            }
                        }
                        // Accent Color Picker
                        Text {
                            text: "Accent Color"
                            color: themeManager.carBlue
                            font.pixelSize: 14
                            font.bold: true
                            topPadding: 10
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            // Accent Color Picker
                            Repeater {
                                model: [
                                    { name: "Blue", color: "#00a8e8" },
                                    { name: "Cyan", color: "#00d4ff" },
                                    { name: "Green", color: "#00ff88" },
                                    { name: "Yellow", color: "#ffd700" },
                                    { name: "Orange", color: "#ff6b35" },
                                    { name: "Red", color: "#ff4444" },
                                    { name: "Pink", color: "#ff69b4" },
                                    { name: "Purple", color: "#9b59b6" },
                                    { name: "White", color: "#ffffff" }
                                ]
                                delegate: Rectangle {
                                    width: 40; height: 40
                                    radius: 20
                                    color: modelData.color
                                    border.color: themeManager.accentColor.toString() === modelData.color ? themeManager.textPrimary : "transparent"
                                    border.width: 2

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            themeManager.setAccentColor(Qt.rgba(0,0,0,1)) // reset
                                            themeManager.setAccentColor(modelData.color)
                                            configManager.setAccentColor(modelData.color)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                // WiFi Section
                Rectangle {
                    Layout.fillWidth: true
                    height: wifiColumn.implicitHeight + 20
                    color: themeManager.bgCard
                    radius: 8
                    Column {
                        id: wifiColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 10
                        spacing: 8
                        Text {
                            text: "WiFi: " + (internalWiFiManager && internalWiFiManager.connected ? "✓ Connected" : "✗ Not Connected")
                            color: internalWiFiManager && internalWiFiManager.connected ? themeManager.statusGreen : themeManager.statusRed
                            font.pixelSize: 16
                            font.bold: true
                        }
                        Text {
                            visible: internalWiFiManager && internalWiFiManager.connected
                            text: "SSID: " + (internalWiFiManager ? internalWiFiManager.ssid : "")
                            color: themeManager.textSecondary
                            font.pixelSize: 14
                        }
                        Text {
                            visible: internalWiFiManager && internalWiFiManager.connected
                            text: "IP: " + (internalWiFiManager ? internalWiFiManager.ipAddress : "")
                            color: themeManager.textSecondary
                            font.pixelSize: 14
                        }
                        Text {
                            visible: internalWiFiManager && internalWiFiManager.connected
                            text: "Signal: " + (internalWiFiManager ? internalWiFiManager.signalStrength : "") + " dBm"
                            color: themeManager.textSecondary
                            font.pixelSize: 14
                        }
                        Row {
                            spacing: 10
                            Button {
                                text: "Scan Networks"
                                onClicked: if (internalWiFiManager) internalWiFiManager.scanNetworks()
                            }
                            Button {
                                text: "Connect to Carputer_ECU"
                                onClicked: if (internalWiFiManager) internalWiFiManager.connectToCarputerECU()
                            }
                            Button {
                                text: "Disconnect"
                                enabled: internalWiFiManager && internalWiFiManager.connected
                                onClicked: if (internalWiFiManager) internalWiFiManager.disconnectNetwork()
                            }
                        }
                        ListView {
                            visible: internalWiFiManager && internalWiFiManager.networks
                            height: 150
                            width: parent.width
                            model: internalWiFiManager ? internalWiFiManager.networks : []
                            delegate: Item {
                                width: parent.width
                                height: 40
                                Row {
                                    anchors.fill: parent
                                    spacing: 10
                                    Text { text: modelData; color: themeManager.textPrimary; verticalAlignment: Text.AlignVCenter }
                                    Button {
                                        text: "Connect"
                                        onClicked: if (internalWiFiManager) internalWiFiManager.connectToNetwork(modelData)
                                    }
                                }
                            }
                        }
                    }
                }
                // Diagnostics Button
                Button {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Run Diagnostics"
                    onClicked: if (debugManager) debugManager.runDiagnostics()
                }
                // Volume Slider
                RowLayout {
                    Layout.fillWidth: true
                    Text { text: "Volume"; color: themeManager.textSecondary; font.pixelSize: 14; Layout.minimumWidth: 80 }
                    Slider {
                        id: volumeSlider
                        Layout.fillWidth: true
                        from: 0; to: 100; value: 50
                        onValueChanged: {
                            if (pressed && mediaManager) mediaManager.setVolume(value / 100.0)
                        }
                    }
                    Text { text: Math.round(volumeSlider.value) + "%"; color: themeManager.textSecondary; font.pixelSize: 12; Layout.minimumWidth: 40 }
                }
                // Car Controller Section
                Rectangle {
                    Layout.fillWidth: true
                    height: controllerColumn.implicitHeight + 20
                    color: themeManager.bgCard
                    radius: 8
                    Column {
                        id: controllerColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 10
                        spacing: 8
                        Text {
                            text: "Body Controller: " + (carControlManager && carControlManager.connected ? "CONNECTED" : "DISCONNECTED")
                            color: carControlManager && carControlManager.connected ? themeManager.statusGreen : themeManager.statusRed
                            font.pixelSize: 16
                            font.bold: true
                        }
                    }
                }
                // Sensor Section
                Rectangle {
                    Layout.fillWidth: true
                    height: sensorColumn.implicitHeight + 20
                    color: themeManager.bgCard
                    radius: 8
                    Column {
                        id: sensorColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 10
                        spacing: 8
                        Text {
                            text: "Sensors: " + (sensorManager && sensorManager.speed !== undefined ? "ACTIVE" : "NO DATA")
                            color: sensorManager && sensorManager.speed !== undefined ? themeManager.statusGreen : themeManager.statusRed
                            font.pixelSize: 16
                            font.bold: true
                        }
                        Text {
                            visible: sensorManager
                            text: "Speed: " + (sensorManager ? sensorManager.speed : 0) + " mph"
                            color: themeManager.textSecondary
                            font.pixelSize: 14
                        }
                        Text {
                            visible: sensorManager
                            text: "Fuel: " + (sensorManager ? sensorManager.fuelLevel : 0) + "%"
                            color: themeManager.textSecondary
                            font.pixelSize: 14
                        }
                    }
                }
                // Power Section
                Rectangle {
                    Layout.fillWidth: true
                    height: powerColumn.implicitHeight + 20
                    color: themeManager.bgCard
                    radius: 8
                    Column {
                        id: powerColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 10
                        spacing: 8
                        Text {
                            text: "System"
                            color: themeManager.carBlue
                            font.pixelSize: 16
                            font.bold: true
                        }
                        Row {
                            spacing: 10
                            Button {
                                text: "Reboot"
                                onClicked: if (systemManager) systemManager.reboot()
                            }
                            Button {
                                text: "Power Off"
                                onClicked: if (systemManager) systemManager.shutdown()
                            }
                        }
                    }
                }
                // Version info
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: "Version: " + appVersion
                    color: themeManager.textSecondary
                    font.pixelSize: 12
                }
            }
        }
    }
}
