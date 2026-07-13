import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
Item {
    id: settingsPage
    anchors.fill: parent
    focus: true
    property int focusIndex: -1
    property int wifiListFocus: 0
    property int dtcListFocus: 0
    property int diskListFocus: 0
    signal navigateToPage(int page)
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
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 58
                    color: themeManager.bgCard
                    radius: 10
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.30)
                    Rectangle {
                        anchors.fill: parent
                        radius: parent.radius
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.14) }
                            GradientStop { position: 0.6; color: "transparent" }
                        }
                    }
                    Text {
                        anchors.centerIn: parent
                        text: "SETTINGS"
                        color: themeManager.carBlue
                        font.pixelSize: 24
                        font.bold: true
                    }
                }
                // Theme Section
                Rectangle {
                    Layout.fillWidth: true
                    height: themeColumn.implicitHeight + 20
                    color: themeManager.bgCard
                    radius: 8
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)
                    ColumnLayout {
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
                                    highlighted: settingsPage.focusIndex === index
                                    Layout.fillWidth: true
                                    background: Rectangle {
                                        color: themeManager.currentTheme === modelData ? themeManager.carBlueDim : themeManager.bgPanel
                                        radius: 6
                                        border.color: settingsPage.focusIndex === index ? themeManager.carBlue : themeManager.carBlueDim
                                        border.width: settingsPage.focusIndex === index ? 2 : 1
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
                                    border.color: themeManager.accentColor.toString() === modelData.color ? themeManager.textPrimary : (settingsPage.focusIndex === (7 + index) ? themeManager.carBlue : "transparent")
                                    border.width: settingsPage.focusIndex === (7 + index) ? 3 : 2

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            themeManager.setAccentColor(Qt.rgba(0,0,0,1))
                                            themeManager.setAccentColor(modelData.color)
                                            configManager.setAccentColor(modelData.color)
                                        }
                                    }
                                }
                            }
                        }
                        // Gauge Border Color
                        Text {
                            text: "Gauge Border"
                            color: themeManager.carBlue
                            font.pixelSize: 14
                            font.bold: true
                            topPadding: 10
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            Repeater {
                                model: [
                                    { label: "Gauge", mode: "gauge" },
                                    { label: "Primary", mode: "primary" },
                                    { label: "Secondary", mode: "secondary" }
                                ]
                                Button {
                                    text: modelData.label
                                    highlighted: settingsPage.focusIndex === (16 + index)
                                    Layout.fillWidth: true
                                    background: Rectangle {
                                        color: themeManager.gaugeBorderMode === modelData.mode ? themeManager.carBlueDim : themeManager.bgPanel
                                        radius: 6
                                        border.color: settingsPage.focusIndex === (16 + index) ? themeManager.carBlue : themeManager.carBlueDim
                                        border.width: settingsPage.focusIndex === (16 + index) ? 2 : 1
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        color: themeManager.textPrimary
                                        font.pixelSize: 14
                                        horizontalAlignment: Text.AlignHCenter
                                    }
                                    onClicked: {
                                        themeManager.setGaugeBorderMode(modelData.mode)
                                        configManager.setValue("gaugeBorderMode", modelData.mode)
                                    }
                                }
                            }
                        }
                        // Preview swatches
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6
                            Rectangle {
                                width: 24; height: 24; radius: 12
                                color: themeManager.gaugeColor
                                Layout.alignment: Qt.AlignHCenter
                            }
                            Text {
                                text: "→"
                                color: themeManager.textSecondary
                                font.pixelSize: 16
                            }
                            Rectangle {
                                width: 24; height: 24; radius: 12
                                color: themeManager.gaugeBorderColor
                                Layout.alignment: Qt.AlignHCenter
                                border.width: 2
                                border.color: themeManager.textSecondary
                            }
                        }
                    }
                }
                // Background Image Section
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: bgColumn.implicitHeight + 20
                    color: themeManager.bgCard
                    radius: 8
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)
                    ColumnLayout {
                        id: bgColumn
                        anchors.left: parent.left; anchors.right: parent.right
                        anchors.margins: 10
                        spacing: 8
                        Text {
                            text: "BACKGROUND IMAGE"
                            color: themeManager.carBlue
                            font.pixelSize: 13; font.bold: true
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            TextField {
                                id: bgPathField
                                Layout.fillWidth: true
                                placeholderText: "/path/to/image.jpg or empty = none"
                                text: configManager.backgroundImage
                                font.pixelSize: 12
                                color: themeManager.textPrimary
                                background: Rectangle {
                                    color: themeManager.bgDark
                                    radius: 4
                                    border.width: 1
                                    border.color: themeManager.carBlueDim
                                }
                                onEditingFinished: {
                                    configManager.setBackgroundImage(text.trim())
                                }
                            }
                            Button {
                                text: "Clear"
                                highlighted: settingsPage.focusIndex === 20
                                onClicked: {
                                    bgPathField.text = ""
                                    configManager.setBackgroundImage("")
                                }
                            }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8
                            Text {
                                text: "Opacity: " + Math.round(configManager.backgroundOpacity * 100) + "%"
                                color: themeManager.textSecondary
                                font.pixelSize: 12
                            }
                            Slider {
                                id: bgOpacitySlider
                                Layout.fillWidth: true
                                from: 0.0; to: 0.5; stepSize: 0.05
                                value: configManager.backgroundOpacity
                                onValueChanged: {
                                    configManager.setBackgroundOpacity(value)
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
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)
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
                                highlighted: settingsPage.focusIndex === 22
                                onClicked: if (internalWiFiManager) internalWiFiManager.scanNetworks()
                            }
                            Button {
                                text: "Connect to Carputer_ECU"
                                highlighted: settingsPage.focusIndex === 23
                                onClicked: if (internalWiFiManager) internalWiFiManager.connectToCarputerECU()
                            }
                            Button {
                                text: "Disconnect"
                                highlighted: settingsPage.focusIndex === 24
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
                                    Text { text: modelData; color: settingsPage.focusIndex === 25 && settingsPage.wifiListFocus === index ? themeManager.carBlue : themeManager.textPrimary; verticalAlignment: Text.AlignVCenter }
                                    Button {
                                        text: "Connect"
                                        highlighted: settingsPage.focusIndex === 25 && settingsPage.wifiListFocus === index
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
                    highlighted: settingsPage.focusIndex === 26
                    onClicked: if (debugManager) debugManager.runDiagnostics()
                }
                // ── ECU DTC Diagnostics Card ──────────────────────────────
                Rectangle {
                    Layout.fillWidth: true
                    height: dtcColumn.implicitHeight + 20
                    color: themeManager.bgCard
                    radius: 8
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carOrange.r, themeManager.carOrange.g, themeManager.carOrange.b, 0.22)
                    Column {
                        id: dtcColumn
                        anchors.left: parent.left; anchors.right: parent.right
                        anchors.top: parent.top; anchors.margins: 10
                        spacing: 8
                        Text {
                            text: "ECU DIAGNOSTICS (DTC)"
                            color: themeManager.carOrange
                            font.pixelSize: 13; font.bold: true
                        }
                        Text {
                            text: dtcManager ? dtcManager.statusText : ""
                            color: themeManager.textSecondary
                            font.pixelSize: 11
                            visible: dtcManager && dtcManager.statusText !== "Ready"
                        }
                        Row {
                            spacing: 8
                            Button {
                                text: "Scan for Codes"
                                highlighted: settingsPage.focusIndex === 27
                                enabled: dtcManager && !dtcManager.busy
                                palette.button: themeManager.carBlue
                                onClicked: if (dtcManager) dtcManager.scanDtc()
                            }
                            Button {
                                text: "Test Mode"
                                highlighted: settingsPage.focusIndex === 28
                                enabled: dtcManager && !dtcManager.busy
                                palette.button: Qt.rgba(themeManager.carOrange.r, themeManager.carOrange.g, themeManager.carOrange.b, 0.5)
                                onClicked: if (dtcManager) dtcManager.scanDtcTestMode()
                            }
                        }
                        Repeater {
                            model: dtcManager ? dtcManager.dtcCodes : []
                            delegate: Item {
                                width: parent.width; height: 36
                                Rectangle {
                                    width: parent.width; height: 34
                                    color: themeManager.bgDark
                                    radius: 4
                                    border.width: settingsPage.focusIndex === 29 && settingsPage.dtcListFocus === index ? 2 : 1
                                    border.color: settingsPage.focusIndex === 29 && settingsPage.dtcListFocus === index ? themeManager.carBlue : Qt.rgba(themeManager.statusRed.r, themeManager.statusRed.g, themeManager.statusRed.b, 0.15)
                                    Row {
                                        anchors.left: parent.left; anchors.right: parent.right
                                        anchors.margins: 8; anchors.verticalCenter: parent.verticalCenter
                                        spacing: 10
                                        Text {
                                            text: "Code " + modelData
                                            color: themeManager.statusRed
                                            font.pixelSize: 14; font.bold: true
                                            width: 70
                                        }
                                        Text {
                                            text: dtcManager ? dtcManager.describeCode(modelData).split("\n")[0].replace("Code " + modelData + "  ", "") : ""
                                            color: themeManager.textPrimary
                                            font.pixelSize: 12
                                            elide: Text.ElideRight
                                            width: parent.width - 100
                                        }
                                    }
                                }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: dtcInfoPopup.code = modelData
                                }
                            }
                        }
                        Text {
                            text: "Clear codes: Remove EFI 15A fuse with ignition OFF for 10+ seconds"
                            color: themeManager.textSecondary
                            font.pixelSize: 10
                            visible: dtcManager && dtcManager.dtcCount > 0
                        }
                    }
                }
                // DTC Info Popup
                Dialog {
                    id: dtcInfoPopup
                    property int code: 0
                    title: "DTC " + code
                    standardButtons: Dialog.Ok
                    Label {
                        text: dtcManager ? dtcManager.describeCode(dtcInfoPopup.code) : ""
                        color: themeManager.textPrimary
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                    }
                }
                // ── DTC History Card ────────────────────────────────────
                Rectangle {
                    Layout.fillWidth: true
                    height: dtcHistoryColumn.implicitHeight + 20
                    color: themeManager.bgCard
                    radius: 8
                    border.width: 1
                    border.color: Qt.rgba(themeManager.statusRed.r, themeManager.statusRed.g, themeManager.statusRed.b, 0.22)
                    Column {
                        id: dtcHistoryColumn
                        anchors.left: parent.left; anchors.right: parent.right
                        anchors.top: parent.top; anchors.margins: 10
                        spacing: 8
                        Text {
                            text: "DTC HISTORY"
                            color: themeManager.statusRed
                            font.pixelSize: 13; font.bold: true
                        }
                        Text {
                            text: "Stored fault codes with timestamps"
                            color: themeManager.textSecondary; font.pixelSize: 10
                        }
                        Repeater {
                            model: dtcManager ? dtcManager.dtcHistory : []
                            delegate: Item {
                                width: parent.width; height: 44
                                Rectangle {
                                    width: parent.width; height: 42
                                    color: themeManager.bgDark; radius: 4
                                    border.width: 1
                                    border.color: Qt.rgba(themeManager.statusRed.r, themeManager.statusRed.g, themeManager.statusRed.b, 0.12)
                                    Row {
                                        anchors.fill: parent; anchors.margins: 8; spacing: 8
                                        Text {
                                            text: "Code " + modelData.code
                                            color: themeManager.statusRed
                                            font.pixelSize: 14; font.bold: true
                                            width: 70
                                        }
                                        Column {
                                            width: parent.width - 160; spacing: 1
                                            Text {
                                                text: modelData.description ? modelData.description.split("\n")[0].replace("Code " + modelData.code + "  ", "") : ""
                                                color: themeManager.textPrimary; font.pixelSize: 11
                                                elide: Text.ElideRight; width: parent.width
                                            }
                                            Text {
                                                text: modelData.timestamp || ""
                                                color: themeManager.textSecondary; font.pixelSize: 9
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        Button {
                            text: "Clear History"
                            highlighted: settingsPage.focusIndex === 30
                            enabled: dtcManager && dtcManager.dtcHistory.length > 0
                            palette.button: themeManager.bgPanel
                            onClicked: if (dtcManager) dtcManager.clearHistory()
                        }
                    }
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
                        onPressedChanged: { if (!pressed) value = Qt.binding(function() { return mediaManager ? mediaManager.volume : 80 }) }
                    }
                    Text { text: mediaManager.volume + "%"; color: themeManager.textSecondary; font.pixelSize: 12; Layout.minimumWidth: 40 }
                }
                // Car Controller Section
                Rectangle {
                    Layout.fillWidth: true
                    height: controllerColumn.implicitHeight + 20
                    color: themeManager.bgCard
                    radius: 8
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)
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
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)
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
                // Engine Profile Section
                Rectangle {
                    Layout.fillWidth: true
                    height: engineProfileColumn.implicitHeight + 20
                    color: themeManager.bgCard
                    radius: 8
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carOrange.r, themeManager.carOrange.g, themeManager.carOrange.b, 0.22)
                    ColumnLayout {
                        id: engineProfileColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 10
                        spacing: 8
                        Text {
                            text: "ENGINE PROFILE"
                            color: themeManager.carOrange
                            font.pixelSize: 16
                            font.bold: true
                        }
                        Text {
                            text: "Current: " + (engineProfile ? engineProfile.engineCode + " \u2014 " + engineProfile.engineName : "None")
                            color: themeManager.textPrimary
                            font.pixelSize: 14
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 6
                            Repeater {
                                model: engineProfile ? engineProfile.profileList : []
                                Button {
                                    text: modelData
                                    Layout.fillWidth: true
                                    highlighted: engineProfile && engineProfile.profileName === modelData
                                    background: Rectangle {
                                        color: engineProfile && engineProfile.profileName === modelData ? themeManager.carOrange : themeManager.bgPanel
                                        radius: 6
                                        border.color: settingsPage.focusIndex === 34 && engineProfile.profileName === modelData ? themeManager.carBlue : themeManager.carOrange
                                        border.width: settingsPage.focusIndex === 34 && engineProfile.profileName === modelData ? 2 : 1
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        color: engineProfile && engineProfile.profileName === modelData ? "#000" : themeManager.textPrimary
                                        font.pixelSize: 12
                                        horizontalAlignment: Text.AlignHCenter
                                    }
                                    onClicked: {
                                        engineProfile.loadProfile(modelData)
                                    }
                                }
                            }
                        }
                    }
                }
                // ── Drivetrain / Tire Size ───────────────────────────────
                Rectangle {
                    Layout.fillWidth: true
                    height: drivetrainColumn.implicitHeight + 20
                    color: themeManager.bgCard
                    radius: 8
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)
                    ColumnLayout {
                        id: drivetrainColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 10
                        spacing: 8
                        Text {
                            text: "DRIVETRAIN"
                            color: themeManager.carBlue
                            font.pixelSize: 16; font.bold: true
                        }
                        Text {
                            text: "Override profile defaults for gear detection"
                            color: themeManager.textSecondary; font.pixelSize: 11
                        }
                        // Tire Width
                        Text { text: "Tire Width (mm)"; color: themeManager.textSecondary; font.pixelSize: 12 }
                        RowLayout {
                            Layout.fillWidth: true; spacing: 4
                            Repeater {
                                model: [185, 195, 205, 215, 225, 235, 245, 255]
                                Button {
                                    text: modelData
                                    Layout.fillWidth: true
                                    highlighted: engineProfile && engineProfile.userTireWidth === modelData
                                    background: Rectangle {
                                        color: engineProfile && engineProfile.userTireWidth === modelData ? themeManager.carBlue : themeManager.bgPanel
                                        radius: 4
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        color: engineProfile && engineProfile.userTireWidth === modelData ? "#fff" : themeManager.textPrimary
                                        font.pixelSize: 11; horizontalAlignment: Text.AlignHCenter
                                    }
                                    onClicked: engineProfile.setUserTireWidth(modelData)
                                }
                            }
                        }
                        // Tire Aspect Ratio
                        Text { text: "Aspect Ratio (%)"; color: themeManager.textSecondary; font.pixelSize: 12 }
                        RowLayout {
                            Layout.fillWidth: true; spacing: 4
                            Repeater {
                                model: [40, 45, 50, 55, 60, 65, 70, 75]
                                Button {
                                    text: modelData
                                    Layout.fillWidth: true
                                    highlighted: engineProfile && engineProfile.userTireAspectRatio === modelData
                                    background: Rectangle {
                                        color: engineProfile && engineProfile.userTireAspectRatio === modelData ? themeManager.carBlue : themeManager.bgPanel
                                        radius: 4
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        color: engineProfile && engineProfile.userTireAspectRatio === modelData ? "#fff" : themeManager.textPrimary
                                        font.pixelSize: 11; horizontalAlignment: Text.AlignHCenter
                                    }
                                    onClicked: engineProfile.setUserTireAspectRatio(modelData)
                                }
                            }
                        }
                        // Rim Diameter
                        Text { text: "Rim Diameter (inches)"; color: themeManager.textSecondary; font.pixelSize: 12 }
                        RowLayout {
                            Layout.fillWidth: true; spacing: 4
                            Repeater {
                                model: [13, 14, 15, 16, 17, 18, 19, 20]
                                Button {
                                    text: modelData
                                    Layout.fillWidth: true
                                    highlighted: engineProfile && engineProfile.userTireRimDiameter === modelData
                                    background: Rectangle {
                                        color: engineProfile && engineProfile.userTireRimDiameter === modelData ? themeManager.carBlue : themeManager.bgPanel
                                        radius: 4
                                    }
                                    contentItem: Text {
                                        text: parent.text
                                        color: engineProfile && engineProfile.userTireRimDiameter === modelData ? "#fff" : themeManager.textPrimary
                                        font.pixelSize: 11; horizontalAlignment: Text.AlignHCenter
                                    }
                                    onClicked: engineProfile.setUserTireRimDiameter(modelData)
                                }
                            }
                        }
                        // Computed info
                        Text {
                            text: engineProfile ? "Tire: " + engineProfile.userTireWidth + "/" + engineProfile.userTireAspectRatio + "R" + engineProfile.userTireRimDiameter + "  •  Circumference: " + engineProfile.tireCircumferenceM.toFixed(3) + "m  •  Final drive: " + engineProfile.finalDrive.toFixed(3) : ""
                            color: themeManager.textPrimary; font.pixelSize: 11
                        }
                        Text {
                            text: engineProfile ? "Gear ratios: " + engineProfile.gearRatios.join(" / ") : ""
                            color: themeManager.textSecondary; font.pixelSize: 10
                        }
                    }
                }

                // ── Advanced Section ───────────────────────────────
                Rectangle {
                    Layout.fillWidth: true
                    height: advancedColumn.implicitHeight + 20
                    color: themeManager.bgCard
                    radius: 8
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)
                    ColumnLayout {
                        id: advancedColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 10
                        spacing: 8
                        Text {
                            text: "ADVANCED"
                            color: themeManager.carBlue
                            font.pixelSize: 16; font.bold: true
                        }
                        Text {
                            text: "Bluetooth, VIN binding, and alarm configuration"
                            color: themeManager.textSecondary; font.pixelSize: 11
                        }
                        Row {
                            Layout.fillWidth: true
                            spacing: 8
                            Rectangle {
                                width: (parent.width - 16) / 3
                                height: 60
                                color: themeManager.bgDark
                                radius: 8
                                border.width: 1
                                border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.30)
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: settingsPage.navigateToPage(8)
                                }
                                Column {
                                    anchors.centerIn: parent
                                    spacing: 2
                                    Text { text: "\u27D0"; color: themeManager.carBlue; font.pixelSize: 20; anchors.horizontalCenter: parent.horizontalCenter }
                                    Text { text: "BLUETOOTH"; color: themeManager.textPrimary; font.pixelSize: 11; font.bold: true; anchors.horizontalCenter: parent.horizontalCenter }
                                }
                            }
                            Rectangle {
                                width: (parent.width - 16) / 3
                                height: 60
                                color: themeManager.bgDark
                                radius: 8
                                border.width: 1
                                border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.30)
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: settingsPage.navigateToPage(9)
                                }
                                Column {
                                    anchors.centerIn: parent
                                    spacing: 2
                                    Text { text: "\u26CA"; color: themeManager.carBlue; font.pixelSize: 20; anchors.horizontalCenter: parent.horizontalCenter }
                                    Text { text: "VIN"; color: themeManager.textPrimary; font.pixelSize: 11; font.bold: true; anchors.horizontalCenter: parent.horizontalCenter }
                                }
                            }
                            Rectangle {
                                width: (parent.width - 16) / 3
                                height: 60
                                color: themeManager.bgDark
                                radius: 8
                                border.width: 1
                                border.color: Qt.rgba(themeManager.carOrange.r, themeManager.carOrange.g, themeManager.carOrange.b, 0.30)
                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: settingsPage.navigateToPage(10)
                                }
                                Column {
                                    anchors.centerIn: parent
                                    spacing: 2
                                    Text { text: "\u26A0"; color: themeManager.carOrange; font.pixelSize: 20; anchors.horizontalCenter: parent.horizontalCenter }
                                    Text { text: "ALARM"; color: themeManager.textPrimary; font.pixelSize: 11; font.bold: true; anchors.horizontalCenter: parent.horizontalCenter }
                                }
                            }
                        }
                    }
                }

                // Power Section
                Rectangle {
                    Layout.fillWidth: true
                    height: powerColumn.implicitHeight + 20
                    color: themeManager.bgCard
                    radius: 8
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)
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
                                highlighted: settingsPage.focusIndex === 35
                                onClicked: if (systemManager) systemManager.reboot()
                            }
                            Button {
                                text: "Power Off"
                                highlighted: settingsPage.focusIndex === 36
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
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)
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
                                highlighted: settingsPage.focusIndex === 37
                                onClicked: installManager.scanDisks()
                            }
                            Button {
                                text: installColumn.selectedDisk
                                       ? ("Install to " + installColumn.selectedDisk)
                                       : "Install"
                                highlighted: settingsPage.focusIndex === 38
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
                                border.color: settingsPage.focusIndex === 39 && settingsPage.diskListFocus === index ? themeManager.carBlue :
                                              (installColumn.selectedDisk === modelData.device
                                               ? themeManager.carBlue : "transparent")
                                border.width: settingsPage.focusIndex === 39 && settingsPage.diskListFocus === index ? 2 :
                                             (installColumn.selectedDisk === modelData.device ? 2 : 0)

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
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)
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
                                highlighted: settingsPage.focusIndex === 40
                                enabled: updateManager && !updateManager.busy
                                onClicked: updateManager.checkForUpdate()
                            }
                            Button {
                                text: "Download & Install"
                                highlighted: settingsPage.focusIndex === 41
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
            property int dialogFocus: 0
            onVisibleChanged: if (visible) dialogFocus = 0
            Keys.onPressed: {
                if (event.key === Qt.Key_Left && dialogFocus > 0) { dialogFocus--; event.accepted = true; return }
                if (event.key === Qt.Key_Right && dialogFocus < 1) { dialogFocus++; event.accepted = true; return }
                if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                    if (dialogFocus === 0) close()
                    else { close(); installManager.installToDisk(installColumn.selectedDisk) }
                    event.accepted = true; return
                }
                if (event.key === Qt.Key_Escape) { close(); event.accepted = true; return }
            }
            Column {
                spacing: 12
                Text {
                    text: "WARNING: Destructive Operation"
                    color: themeManager.statusRed
                    font.pixelSize: 18; font.bold: true
                }
                Text {
                    text: "Installing to " + installColumn.selectedDisk
                          + " will ERASE ALL DATA on that disk.\n\n"
                          + "This cannot be undone. Continue?"
                    color: themeManager.textPrimary; font.pixelSize: 14
                    width: 400; wrapMode: Text.WordWrap
                }
                Row {
                    spacing: 10
                    Button {
                        text: "Cancel"
                        highlighted: confirmDialog.dialogFocus === 0
                        onClicked: confirmDialog.close()
                    }
                    Button {
                        text: "Proceed with Installation"
                        highlighted: confirmDialog.dialogFocus === 1
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
            property int dialogFocus: 0
            onVisibleChanged: if (visible) dialogFocus = 0
            Keys.onPressed: {
                if (event.key === Qt.Key_Left && dialogFocus > 0) { dialogFocus--; event.accepted = true; return }
                if (event.key === Qt.Key_Right && dialogFocus < 1) { dialogFocus++; event.accepted = true; return }
                if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                    if (dialogFocus === 0) close()
                    else if (resultDialog.success) installManager.rebootNow()
                    event.accepted = true; return
                }
                if (event.key === Qt.Key_Escape) { close(); event.accepted = true; return }
            }

            Column {
                spacing: 12
                Text {
                    text: resultDialog.success
                          ? "Installation Complete!"
                          : "Installation Failed"
                    color: resultDialog.success
                           ? themeManager.statusGreen
                           : themeManager.statusRed
                    font.pixelSize: 18; font.bold: true
                }
                Text {
                    text: resultDialog.resultMessage
                    color: themeManager.textPrimary; font.pixelSize: 14
                    width: 400; wrapMode: Text.WordWrap
                }
                Row {
                    spacing: 10
                    Button {
                        text: "Later"
                        highlighted: resultDialog.dialogFocus === 0
                        onClicked: resultDialog.close()
                    }
                    Button {
                        visible: resultDialog.success
                        text: "Reboot Now"
                        highlighted: resultDialog.dialogFocus === 1
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

            Keys.onPressed: {
                if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter || event.key === Qt.Key_Escape) {
                    close(); event.accepted = true; return
                }
            }

            Column {
                spacing: 12
                Text {
                    text: updateResultDialog.success
                          ? "Update Complete!"
                          : "Update Failed"
                    color: updateResultDialog.success
                           ? themeManager.statusGreen
                           : themeManager.statusRed
                    font.pixelSize: 18; font.bold: true
                }
                Text {
                    text: updateResultDialog.resultMessage
                    color: themeManager.textPrimary; font.pixelSize: 14
                    width: 400; wrapMode: Text.WordWrap
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
    function activateCurrentFocus() {
        var dlg = confirmDialog.visible ? 0 : (resultDialog.visible ? 1 : (updateResultDialog.visible ? 2 : -1))
        if (dlg === 0) {
            if (confirmDialog.dialogFocus === 0) confirmDialog.close()
            else { confirmDialog.close(); installManager.installToDisk(installColumn.selectedDisk) }
            return
        }
        if (dlg === 1) {
            if (resultDialog.dialogFocus === 0) resultDialog.close()
            else installManager.rebootNow()
            return
        }
        if (dlg === 2) { updateResultDialog.close(); return }
        if (focusIndex < 7) {
            var themes = ["Dark", "Light", "Blue", "Red", "Green", "Purple", "Orange"]
            themeManager.setCurrentTheme(themes[focusIndex])
            configManager.setTheme(themes[focusIndex])
        } else if (focusIndex < 16) {
            var colors = ["#00a8e8", "#00d4ff", "#00ff88", "#ffd700", "#ff6b35", "#ff4444", "#ff69b4", "#9b59b6", "#ffffff"]
            themeManager.setAccentColor(colors[focusIndex - 7])
            configManager.setAccentColor(colors[focusIndex - 7])
        } else if (focusIndex < 19) {
            var modes = ["gauge", "primary", "secondary"]
            themeManager.setGaugeBorderMode(modes[focusIndex - 16])
            configManager.setValue("gaugeBorderMode", modes[focusIndex - 16])
        } else if (focusIndex === 19) { bgPathField.forceActiveFocus()
        } else if (focusIndex === 20) { bgPathField.text = ""; configManager.setBackgroundImage("")
        } else if (focusIndex === 22 && internalWiFiManager) { internalWiFiManager.scanNetworks()
        } else if (focusIndex === 23 && internalWiFiManager) { internalWiFiManager.connectToCarputerECU()
        } else if (focusIndex === 24 && internalWiFiManager) { internalWiFiManager.disconnectNetwork()
        } else if (focusIndex === 25 && internalWiFiManager && internalWiFiManager.networks.length > 0) { internalWiFiManager.connectToNetwork(internalWiFiManager.networks[wifiListFocus])
        } else if (focusIndex === 26 && debugManager) { debugManager.runDiagnostics()
        } else if (focusIndex === 27 && dtcManager) { dtcManager.scanDtc()
        } else if (focusIndex === 28 && dtcManager) { dtcManager.scanDtcTestMode()
        } else if (focusIndex === 29 && dtcManager && dtcManager.dtcCodes.length > 0) { dtcInfoPopup.code = dtcManager.dtcCodes[dtcListFocus]
        } else if (focusIndex === 30 && dtcManager) { dtcManager.clearHistory()
        } else if (focusIndex === 34 && engineProfile) {
            var profiles = engineProfile.profileList
            var currentIdx = profiles.indexOf(engineProfile.profileName)
            var nextIdx = (currentIdx + 1) % profiles.length
            engineProfile.loadProfile(profiles[nextIdx])
        } else if (focusIndex === 35 && systemManager) { systemManager.reboot()
        } else if (focusIndex === 36 && systemManager) { systemManager.shutdown()
        } else if (focusIndex === 37) { installManager.scanDisks()
        } else if (focusIndex === 38) { confirmDialog.open()
        } else if (focusIndex === 39 && installManager.disks.length > 0) { installColumn.selectedDisk = installManager.disks[diskListFocus].device
        } else if (focusIndex === 40) { updateManager.checkForUpdate()
        } else if (focusIndex === 41 && updateManager.updateAvailable) { updateManager.applyNetworkUpdate() }
    }

    function navigateLeft()  { if (focusIndex > 0) focusIndex-- }
    function navigateRight() {
        var dlg = confirmDialog.visible ? 0 : (resultDialog.visible ? 1 : (updateResultDialog.visible ? 2 : -1))
        if (dlg === 0 && focusIndex < 1) { focusIndex++; return }
        if (dlg === 1 && focusIndex < 1) { focusIndex++; return }
        if (dlg === 0 || dlg === 1 || dlg === 2) return
        if (focusIndex < 41) focusIndex++
    }
    function navigateUp() {
        if (focusIndex <= 6) { focusIndex = 0; return true }
        if (focusIndex <= 15) { focusIndex = 0; return true }
        if (focusIndex <= 18) { focusIndex = 7; return true }
        if (focusIndex <= 20) { focusIndex = 16; return true }
        if (focusIndex <= 25 && focusIndex !== 25) { focusIndex = 19; return true }
        if (focusIndex === 25) {
            if (wifiListFocus > 0) { wifiListFocus--; return true }
            focusIndex = 19; return true
        }
        if (focusIndex === 26) { focusIndex = 22; return true }
        if (focusIndex === 29) {
            if (dtcListFocus > 0) { dtcListFocus--; return true }
            focusIndex = 26; return true
        }
        if (focusIndex <= 30 && focusIndex !== 29) { focusIndex = 26; return true }
        if (focusIndex === 34) { focusIndex = 30; return true }
        if (focusIndex <= 36) { focusIndex = 34; return true }
        if (focusIndex === 39) {
            if (diskListFocus > 0) { diskListFocus--; return true }
            focusIndex = 36; return true
        }
        if (focusIndex <= 41 && focusIndex !== 39) { focusIndex = 37; return true }
        return true
    }
    function navigateDown() {
        if (focusIndex <= 6) { focusIndex = 7; return true }
        if (focusIndex <= 15) { focusIndex = 16; return true }
        if (focusIndex <= 18) { focusIndex = 19; return true }
        if (focusIndex <= 20) { focusIndex = 22; return true }
        if (focusIndex <= 22) { focusIndex = 23; return true }
        if (focusIndex === 23) { focusIndex = 24; return true }
        if (focusIndex === 24) { focusIndex = 25; wifiListFocus = 0; return true }
        if (focusIndex === 25) {
            var maxWifi = internalWiFiManager ? Math.max(0, internalWiFiManager.networks.length - 1) : 0
            if (wifiListFocus < maxWifi) { wifiListFocus++; return true }
            wifiListFocus = 0; focusIndex = 26; return true
        }
        if (focusIndex === 26) { focusIndex = 27; return true }
        if (focusIndex === 27) { focusIndex = 28; return true }
        if (focusIndex === 28) { focusIndex = 29; dtcListFocus = 0; return true }
        if (focusIndex === 29) {
            var maxDtc = dtcManager ? Math.max(0, dtcManager.dtcCodes.length - 1) : 0
            if (dtcListFocus < maxDtc) { dtcListFocus++; return true }
            dtcListFocus = 0; focusIndex = 30; return true
        }
        if (focusIndex === 30) { focusIndex = 34; return true }
        if (focusIndex === 34) { focusIndex = 35; return true }
        if (focusIndex === 35) { focusIndex = 36; return true }
        if (focusIndex === 36) { focusIndex = 37; return true }
        if (focusIndex === 37) { focusIndex = 38; return true }
        if (focusIndex === 38) { focusIndex = 39; diskListFocus = 0; return true }
        if (focusIndex === 39) {
            var maxDisk = installManager ? Math.max(0, installManager.disks.length - 1) : 0
            if (diskListFocus < maxDisk) { diskListFocus++; return true }
            diskListFocus = 0; focusIndex = 40; return true
        }
        if (focusIndex === 40) { focusIndex = 41; return true }
        return false
    }

    Keys.onPressed: {
        var dlg = confirmDialog.visible ? 0 : (resultDialog.visible ? 1 : (updateResultDialog.visible ? 2 : -1))
        if (event.key === Qt.Key_Left) {
            if (focusIndex > 0) focusIndex--
            event.accepted = true; return
        }
        if (event.key === Qt.Key_Right) {
            if (dlg === 0 && focusIndex < 1) { focusIndex++; event.accepted = true; return }
            if (dlg === 1 && focusIndex < 1) { focusIndex++; event.accepted = true; return }
            if (dlg === 0 || dlg === 1 || dlg === 2) return
            if (focusIndex < 41) focusIndex++
            event.accepted = true; return
        }
        if (event.key === Qt.Key_Up) {
            navigateUp()
            event.accepted = true; return
        }
        if (event.key === Qt.Key_Down) {
            navigateDown()
            event.accepted = true; return
        }
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            if (dlg === 0) {
                if (confirmDialog.dialogFocus === 0) confirmDialog.close()
                else { confirmDialog.close(); installManager.installToDisk(installColumn.selectedDisk) }
                event.accepted = true; return
            }
            if (dlg === 1) {
                if (resultDialog.dialogFocus === 0) resultDialog.close()
                else installManager.rebootNow()
                event.accepted = true; return
            }
            if (dlg === 2) { updateResultDialog.close(); event.accepted = true; return }
            if (focusIndex < 7) {
                var themes = ["Dark", "Light", "Blue", "Red", "Green", "Purple", "Orange"]
                themeManager.setCurrentTheme(themes[focusIndex])
                configManager.setTheme(themes[focusIndex])
            } else if (focusIndex < 16) {
                var colors = ["#00a8e8", "#00d4ff", "#00ff88", "#ffd700", "#ff6b35", "#ff4444", "#ff69b4", "#9b59b6", "#ffffff"]
                themeManager.setAccentColor(colors[focusIndex - 7])
                configManager.setAccentColor(colors[focusIndex - 7])
            } else if (focusIndex < 19) {
                var modes = ["gauge", "primary", "secondary"]
                themeManager.setGaugeBorderMode(modes[focusIndex - 16])
                configManager.setValue("gaugeBorderMode", modes[focusIndex - 16])
            } else if (focusIndex === 19) { bgPathField.forceActiveFocus()
            } else if (focusIndex === 20) { bgPathField.text = ""; configManager.setBackgroundImage("")
            } else if (focusIndex === 22 && internalWiFiManager) { internalWiFiManager.scanNetworks()
            } else if (focusIndex === 23 && internalWiFiManager) { internalWiFiManager.connectToCarputerECU()
            } else if (focusIndex === 24 && internalWiFiManager) { internalWiFiManager.disconnectNetwork()
        } else if (focusIndex === 25 && internalWiFiManager && internalWiFiManager.networks.length > 0) { internalWiFiManager.connectToNetwork(internalWiFiManager.networks[wifiListFocus])
            } else if (focusIndex === 26 && debugManager) { debugManager.runDiagnostics()
            } else if (focusIndex === 27 && dtcManager) { dtcManager.scanDtc()
            } else if (focusIndex === 28 && dtcManager) { dtcManager.scanDtcTestMode()
        } else if (focusIndex === 29 && dtcManager && dtcManager.dtcCodes.length > 0) { dtcInfoPopup.code = dtcManager.dtcCodes[dtcListFocus]
            } else if (focusIndex === 30 && dtcManager) { dtcManager.clearHistory()
            } else if (focusIndex === 34 && engineProfile) {
                var profiles = engineProfile.profileList
                var currentIdx = profiles.indexOf(engineProfile.profileName)
                var nextIdx = (currentIdx + 1) % profiles.length
                engineProfile.loadProfile(profiles[nextIdx])
            } else if (focusIndex === 35 && systemManager) { systemManager.reboot()
            } else if (focusIndex === 36 && systemManager) { systemManager.shutdown()
            } else if (focusIndex === 37) { installManager.scanDisks()
            } else if (focusIndex === 38) { confirmDialog.open()
        } else if (focusIndex === 39 && installManager.disks.length > 0) { installColumn.selectedDisk = installManager.disks[diskListFocus].device
            } else if (focusIndex === 40) { updateManager.checkForUpdate()
            } else if (focusIndex === 41 && updateManager.updateAvailable) { updateManager.applyNetworkUpdate() }
            event.accepted = true; return
        }
    }
}
