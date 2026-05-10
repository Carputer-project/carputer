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
                        from: 0; to: 100
                        value: mediaManager ? mediaManager.volume : 80
                        onMoved: mediaManager.setVolume(Math.round(value))
                    }
                    Text { text: mediaManager.volume + "%"; color: themeManager.textSecondary; font.pixelSize: 12; Layout.minimumWidth: 40 }
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
                // Install OS Section
                Rectangle {
                    Layout.fillWidth: true
                    height: installColumn.implicitHeight + 20
                    color: themeManager.bgCard
                    radius: 8
                    Column {
                        id: installColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 10
                        spacing: 8

                        property string selectedDisk: ""

                        Text {
                            text: "Install OS to Disk"
                            color: themeManager.carBlue
                            font.pixelSize: 16
                            font.bold: true
                        }

                        Row {
                            spacing: 10
                            Button {
                                text: "Scan Disks"
                                onClicked: installManager.scanDisks()
                            }
                            Button {
                                text: installColumn.selectedDisk
                                       ? ("Install to " + installColumn.selectedDisk)
                                       : "Install"
                                enabled: installColumn.selectedDisk !== "" && !installManager.busy
                                onClicked: confirmDialog.open()
                            }
                        }

                        ListView {
                            height: Math.min(200, contentHeight)
                            width: parent.width
                            model: installManager.disks
                            visible: installManager.disks.length > 0

                            delegate: Rectangle {
                                width: parent.width
                                height: 50
                                color: installColumn.selectedDisk === modelData.device
                                       ? themeManager.carBlueDim : "transparent"
                                radius: 4
                                border.color: installColumn.selectedDisk === modelData.device
                                              ? themeManager.carBlue : "transparent"
                                border.width: installColumn.selectedDisk === modelData.device ? 2 : 0

                                Text {
                                    anchors.left: parent.left
                                    anchors.leftMargin: 10
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: modelData.device
                                          + "  " + modelData.size
                                          + "  " + modelData.model
                                    color: themeManager.textPrimary
                                    font.pixelSize: 13
                                    elide: Text.ElideRight
                                    width: parent.width - 20
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: installColumn.selectedDisk = modelData.device
                                }
                            }
                        }

                        Text {
                            visible: installManager.disks.length === 0
                            text: "No disks found. Tap 'Scan Disks'."
                            color: themeManager.textSecondary
                            font.pixelSize: 14
                        }

                        Rectangle {
                            visible: installManager.busy
                            width: parent.width
                            height: 50
                            color: "transparent"
                            Column {
                                spacing: 4
                                ProgressBar {
                                    value: installManager.progress / 100
                                    width: installColumn.width - 20
                                }
                                Text {
                                    text: installManager.progress
                                          + "% - " + installManager.statusText
                                    color: themeManager.textSecondary
                                    font.pixelSize: 12
                                }
                            }
                        }
                    }
                }
                // Updates Section
                Rectangle {
                    Layout.fillWidth: true
                    height: updateColumn.implicitHeight + 20
                    color: themeManager.bgCard
                    radius: 8
                    Column {
                        id: updateColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 10
                        spacing: 8
                        Text {
                            text: "Updates"
                            color: themeManager.carBlue
                            font.pixelSize: 16
                            font.bold: true
                        }
                        Text {
                            text: "Current: " + (updateManager ? updateManager.currentVersion : "unknown")
                            color: themeManager.textSecondary
                            font.pixelSize: 14
                        }
                        Text {
                            text: "Latest: " + (updateManager && updateManager.serverVersion ? updateManager.serverVersion : "\u2014")
                            color: updateManager && updateManager.updateAvailable ? themeManager.statusGreen : themeManager.textSecondary
                            font.pixelSize: 14
                        }
                        Row {
                            spacing: 10
                            Button {
                                text: updateManager && updateManager.busy ? "Checking..." : "Check for Updates"
                                enabled: updateManager && !updateManager.busy
                                onClicked: updateManager.checkForUpdate()
                            }
                            Button {
                                text: "Download & Install"
                                visible: updateManager && updateManager.updateAvailable && !updateManager.busy
                                onClicked: updateManager.applyNetworkUpdate()
                            }
                        }
                        Rectangle {
                            visible: updateManager && updateManager.busy
                            width: parent.width
                            height: 50
                            color: "transparent"
                            Column {
                                spacing: 4
                                ProgressBar {
                                    value: updateManager ? updateManager.progress / 100 : 0
                                    width: updateColumn.width - 20
                                }
                                Text {
                                    text: updateManager ? updateManager.status : ""
                                    color: themeManager.textSecondary
                                    font.pixelSize: 12
                                }
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

        // Confirmation Dialog
        Dialog {
            id: confirmDialog
            modal: true
            standardButtons: Dialog.NoButton
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            Column {
                spacing: 12
                Text {
                    text: "WARNING: Destructive Operation"
                    color: themeManager.statusRed
                    font.pixelSize: 18
                    font.bold: true
                }
                Text {
                    text: "Installing to " + installColumn.selectedDisk
                          + " will ERASE ALL DATA on that disk.\n\n"
                          + "This cannot be undone. Continue?"
                    color: themeManager.textPrimary
                    font.pixelSize: 14
                    width: 400
                    wrapMode: Text.WordWrap
                }
                Row {
                    spacing: 10
                    Button {
                        text: "Cancel"
                        onClicked: confirmDialog.close()
                    }
                    Button {
                        text: "Proceed with Installation"
                        onClicked: {
                            confirmDialog.close()
                            installManager.installToDisk(installColumn.selectedDisk)
                        }
                    }
                }
            }
        }

        // Result Dialog
        Dialog {
            id: resultDialog
            modal: true
            standardButtons: Dialog.NoButton
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2

            property bool success: false
            property string resultMessage: ""

            Column {
                spacing: 12
                Text {
                    text: resultDialog.success
                          ? "Installation Complete!"
                          : "Installation Failed"
                    color: resultDialog.success
                           ? themeManager.statusGreen
                           : themeManager.statusRed
                    font.pixelSize: 18
                    font.bold: true
                }
                Text {
                    text: resultDialog.resultMessage
                    color: themeManager.textPrimary
                    font.pixelSize: 14
                    width: 400
                    wrapMode: Text.WordWrap
                }
                Row {
                    spacing: 10
                    Button {
                        text: "Later"
                        onClicked: resultDialog.close()
                    }
                    Button {
                        visible: resultDialog.success
                        text: "Reboot Now"
                        onClicked: installManager.rebootNow()
                    }
                }
            }
        }

        Connections {
            target: installManager
            onInstallComplete: {
                resultDialog.success = success
                resultDialog.resultMessage = message
                resultDialog.open()
            }
        }

        // Update Result Dialog
        Dialog {
            id: updateResultDialog
            modal: true
            standardButtons: Dialog.NoButton
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2

            property bool success: false
            property string resultMessage: ""

            Column {
                spacing: 12
                Text {
                    text: updateResultDialog.success
                          ? "Update Complete!"
                          : "Update Failed"
                    color: updateResultDialog.success
                           ? themeManager.statusGreen
                           : themeManager.statusRed
                    font.pixelSize: 18
                    font.bold: true
                }
                Text {
                    text: updateResultDialog.resultMessage
                    color: themeManager.textPrimary
                    font.pixelSize: 14
                    width: 400
                    wrapMode: Text.WordWrap
                }
                Row {
                    spacing: 10
                    Button {
                        text: "OK"
                        onClicked: updateResultDialog.close()
                    }
                }
            }
        }

        Connections {
            target: updateManager
            onUpdateComplete: {
                updateResultDialog.success = success
                updateResultDialog.resultMessage = message
                updateResultDialog.open()
            }
        }
    }
}
