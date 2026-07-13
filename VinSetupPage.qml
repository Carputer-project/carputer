import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: vinSetupPage
    focus: true

    property int focusIndex: 0
    property int focusCount: 10

    function navigateUp() { focusIndex = Math.max(0, focusIndex - 1); }
    function navigateDown() { focusIndex = Math.min(focusCount - 1, focusIndex + 1); }
    function activateCurrentFocus() {
        if (focusIndex === 0) vinInput.forceActiveFocus();
        else if (focusIndex === 1) bindBodyBtn.clicked();
        else if (focusIndex === 2) unbindBodyBtn.clicked();
        else if (focusIndex === 3) bindSensorBtn.clicked();
        else if (focusIndex === 4) unbindSensorBtn.clicked();
        else if (focusIndex === 5) bindMeshBtn.clicked();
        else if (focusIndex === 6) unbindMeshBtn.clicked();
        else if (focusIndex === 7) bindRemoteBtn.clicked();
        else if (focusIndex === 8) unbindRemoteBtn.clicked();
        else if (focusIndex === 9) unbindAllBtn.clicked();
    }
    function handleEscape() { root.activePage = 5; }

    Keys.onPressed: {
        if (event.key === Qt.Key_Up) { navigateUp(); event.accepted = true; }
        else if (event.key === Qt.Key_Down) { navigateDown(); event.accepted = true; }
        else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) { activateCurrentFocus(); event.accepted = true; }
        else if (event.key === Qt.Key_Escape) { handleEscape(); event.accepted = true; }
    }

    Rectangle {
        anchors.fill: parent
        color: themeManager.bgDark

        Flickable {
            anchors.fill: parent
            contentHeight: mainColumn.implicitHeight + 40
            clip: true
            flickableDirection: Flickable.VerticalFlick

            ColumnLayout {
                id: mainColumn
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.margins: 20
                spacing: 15

                // ── Header ──
                Rectangle {
                    Layout.fillWidth: true
                    height: 50
                    color: themeManager.bgCard
                    radius: 10
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.3)
                    Text {
                        anchors.centerIn: parent
                        text: "MODULE BINDING"
                        color: themeManager.carBlue
                        font.pixelSize: 24
                        font.bold: true
                    }
                }

                // ── Status ──
                Rectangle {
                    Layout.fillWidth: true
                    height: vinManager.statusText.length > 0 ? 36 : 0
                    visible: vinManager.statusText.length > 0
                    color: themeManager.bgCard
                    radius: 6
                    border.width: 1
                    border.color: vinManager.busy ? Qt.rgba(themeManager.carOrange.r, themeManager.carOrange.g, themeManager.carOrange.b, 0.5)
                                                 : Qt.rgba(themeManager.statusGreen.r, themeManager.statusGreen.g, themeManager.statusGreen.b, 0.5)
                    Text {
                        anchors.centerIn: parent
                        text: vinManager.statusText
                        color: vinManager.busy ? themeManager.carOrange : themeManager.statusGreen
                        font.pixelSize: 14
                        font.bold: true
                    }
                }

                // ── VIN Input ──
                Rectangle {
                    Layout.fillWidth: true
                    height: 100
                    color: themeManager.bgCard
                    radius: 8
                    border.width: focusIndex === 0 ? 2 : 1
                    border.color: focusIndex === 0 ? themeManager.carBlue : Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)

                    Column {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 12
                        spacing: 8

                        Text {
                            text: "VEHICLE IDENTIFICATION NUMBER"
                            color: themeManager.carBlue
                            font.pixelSize: 13
                            font.bold: true
                        }

                        TextInput {
                            id: vinInput
                            width: parent.width
                            color: themeManager.textPrimary
                            font.pixelSize: 20
                            font.family: "Monospace"
                            font.bold: true
                            maximumLength: 17
                            clip: true
                            focus: focusIndex === 0
                            text: vinManager.vin

                            onAccepted: {
                                vinManager.vin = text;
                                focusIndex = 1;
                            }

                            Rectangle {
                                anchors.fill: parent
                                color: "transparent"
                                border.width: 1
                                border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.3)
                            }

                            Text {
                                anchors.baseline: parent.baseline
                                text: "JT2AE94S7S0012345"
                                color: Qt.rgba(themeManager.textSecondary.r, themeManager.textSecondary.g, themeManager.textSecondary.b, 0.3)
                                font.pixelSize: 20
                                font.family: "Monospace"
                                visible: parent.text.length === 0
                            }
                        }

                        Text {
                            text: "17 characters — no I, O, Q"
                            color: themeManager.textSecondary
                            font.pixelSize: 11
                        }
                    }
                }

                // ── Body Controller ──
                Rectangle {
                    Layout.fillWidth: true
                    height: bodyColumn.implicitHeight + 20
                    color: themeManager.bgCard
                    radius: 8
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)

                    Column {
                        id: bodyColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 12
                        spacing: 8

                        RowLayout {
                            width: parent.width
                            Text {
                                text: "BODY CONTROLLER"
                                color: themeManager.carBlue
                                font.pixelSize: 13
                                font.bold: true
                                Layout.fillWidth: true
                            }
                            Text {
                                text: vinManager.bodyBound ? "BOUND" : "UNBOUND"
                                color: vinManager.bodyBound ? themeManager.statusGreen : themeManager.statusRed
                                font.pixelSize: 12
                                font.bold: true
                            }
                        }

                        RowLayout {
                            spacing: 10
                            visible: vinManager.bodyBound
                            Text {
                                text: "ID: " + vinManager.bodyModuleId
                                color: themeManager.textSecondary
                                font.pixelSize: 11
                                font.family: "Monospace"
                                Layout.fillWidth: true
                            }
                            Text {
                                text: "MAC: " + vinManager.bodyMac
                                color: themeManager.textSecondary
                                font.pixelSize: 11
                                font.family: "Monospace"
                            }
                        }

                        RowLayout {
                            spacing: 10
                            Button {
                                id: bindBodyBtn
                                text: vinManager.bodyBound ? "REBIND" : "BIND"
                                enabled: !vinManager.busy && vinManager.vin.length === 17
                                highlighted: focusIndex === 1
                                Layout.fillWidth: true
                                background: Rectangle {
                                    color: bindBodyBtn.highlighted ? themeManager.carBlue : themeManager.bgPanel
                                    radius: 6
                                }
                                contentItem: Text {
                                    text: bindBodyBtn.text
                                    color: themeManager.textPrimary
                                    font.pixelSize: 14
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                onClicked: vinManager.bindBody()
                            }
                            Button {
                                id: unbindBodyBtn
                                text: "UNBIND"
                                enabled: !vinManager.busy && vinManager.bodyBound
                                highlighted: focusIndex === 2
                                Layout.fillWidth: true
                                background: Rectangle {
                                    color: unbindBodyBtn.highlighted ? themeManager.statusRed : themeManager.bgPanel
                                    radius: 6
                                }
                                contentItem: Text {
                                    text: unbindBodyBtn.text
                                    color: themeManager.textPrimary
                                    font.pixelSize: 14
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                onClicked: vinManager.unbindBody()
                            }
                        }
                    }
                }

                // ── Sensor Module ──
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
                        anchors.margins: 12
                        spacing: 8

                        RowLayout {
                            width: parent.width
                            Text {
                                text: "SENSOR MODULE"
                                color: themeManager.carBlue
                                font.pixelSize: 13
                                font.bold: true
                                Layout.fillWidth: true
                            }
                            Text {
                                text: vinManager.sensorBound ? "BOUND" : "UNBOUND"
                                color: vinManager.sensorBound ? themeManager.statusGreen : themeManager.statusRed
                                font.pixelSize: 12
                                font.bold: true
                            }
                        }

                        RowLayout {
                            spacing: 10
                            visible: vinManager.sensorBound
                            Text {
                                text: "ID: " + vinManager.sensorModuleId
                                color: themeManager.textSecondary
                                font.pixelSize: 11
                                font.family: "Monospace"
                                Layout.fillWidth: true
                            }
                            Text {
                                text: "MAC: " + vinManager.sensorMac
                                color: themeManager.textSecondary
                                font.pixelSize: 11
                                font.family: "Monospace"
                            }
                        }

                        RowLayout {
                            spacing: 10
                            Button {
                                id: bindSensorBtn
                                text: vinManager.sensorBound ? "REBIND" : "BIND"
                                enabled: !vinManager.busy && vinManager.vin.length === 17
                                highlighted: focusIndex === 3
                                Layout.fillWidth: true
                                background: Rectangle {
                                    color: bindSensorBtn.highlighted ? themeManager.carBlue : themeManager.bgPanel
                                    radius: 6
                                }
                                contentItem: Text {
                                    text: bindSensorBtn.text
                                    color: themeManager.textPrimary
                                    font.pixelSize: 14
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                onClicked: vinManager.bindSensor()
                            }
                            Button {
                                id: unbindSensorBtn
                                text: "UNBIND"
                                enabled: !vinManager.busy && vinManager.sensorBound
                                highlighted: focusIndex === 4
                                Layout.fillWidth: true
                                background: Rectangle {
                                    color: unbindSensorBtn.highlighted ? themeManager.statusRed : themeManager.bgPanel
                                    radius: 6
                                }
                                contentItem: Text {
                                    text: unbindSensorBtn.text
                                    color: themeManager.textPrimary
                                    font.pixelSize: 14
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                onClicked: vinManager.unbindSensor()
                            }
                        }
                    }
                }

                // ── Mesh Module (placeholder) ──
                Rectangle {
                    Layout.fillWidth: true
                    height: meshColumn.implicitHeight + 20
                    color: themeManager.bgCard
                    radius: 8
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)

                    Column {
                        id: meshColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 12
                        spacing: 8

                        RowLayout {
                            width: parent.width
                            Text {
                                text: "MESH MODULE"
                                color: themeManager.carBlue
                                font.pixelSize: 13
                                font.bold: true
                                Layout.fillWidth: true
                            }
                            Text {
                                text: vinManager.meshBound ? "BOUND" : "UNBOUND"
                                color: vinManager.meshBound ? themeManager.statusGreen : themeManager.statusRed
                                font.pixelSize: 12
                                font.bold: true
                            }
                        }

                        Text {
                            text: "V2V mesh — hardware not yet available"
                            color: themeManager.textSecondary
                            font.pixelSize: 11
                            visible: !vinManager.meshBound
                        }

                        RowLayout {
                            spacing: 10
                            visible: vinManager.meshBound
                            Text {
                                text: "ID: " + vinManager.meshModuleId
                                color: themeManager.textSecondary
                                font.pixelSize: 11
                                font.family: "Monospace"
                                Layout.fillWidth: true
                            }
                        }

                        RowLayout {
                            spacing: 10
                            Button {
                                id: bindMeshBtn
                                text: vinManager.meshBound ? "REBIND" : "BIND"
                                enabled: !vinManager.busy && vinManager.vin.length === 17
                                highlighted: focusIndex === 5
                                Layout.fillWidth: true
                                background: Rectangle {
                                    color: bindMeshBtn.highlighted ? themeManager.carBlue : themeManager.bgPanel
                                    radius: 6
                                }
                                contentItem: Text {
                                    text: bindMeshBtn.text
                                    color: themeManager.textPrimary
                                    font.pixelSize: 14
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                onClicked: vinManager.bindMesh()
                            }
                            Button {
                                id: unbindMeshBtn
                                text: "UNBIND"
                                enabled: !vinManager.busy && vinManager.meshBound
                                highlighted: focusIndex === 6
                                Layout.fillWidth: true
                                background: Rectangle {
                                    color: unbindMeshBtn.highlighted ? themeManager.statusRed : themeManager.bgPanel
                                    radius: 6
                                }
                                contentItem: Text {
                                    text: unbindMeshBtn.text
                                    color: themeManager.textPrimary
                                    font.pixelSize: 14
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                onClicked: vinManager.unbindMesh()
                            }
                        }
                    }
                }

                // ── Remote Fob (placeholder) ──
                Rectangle {
                    Layout.fillWidth: true
                    height: remoteColumn.implicitHeight + 20
                    color: themeManager.bgCard
                    radius: 8
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)

                    Column {
                        id: remoteColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 12
                        spacing: 8

                        RowLayout {
                            width: parent.width
                            Text {
                                text: "REMOTE KEYFOB"
                                color: themeManager.carBlue
                                font.pixelSize: 13
                                font.bold: true
                                Layout.fillWidth: true
                            }
                            Text {
                                text: vinManager.remoteBound ? "BOUND" : "UNBOUND"
                                color: vinManager.remoteBound ? themeManager.statusGreen : themeManager.statusRed
                                font.pixelSize: 12
                                font.bold: true
                            }
                        }

                        Text {
                            text: "ESP-NOW fob — hardware not yet available"
                            color: themeManager.textSecondary
                            font.pixelSize: 11
                            visible: !vinManager.remoteBound
                        }

                        RowLayout {
                            spacing: 10
                            visible: vinManager.remoteBound
                            Text {
                                text: "ID: " + vinManager.remoteModuleId
                                color: themeManager.textSecondary
                                font.pixelSize: 11
                                font.family: "Monospace"
                                Layout.fillWidth: true
                            }
                        }

                        RowLayout {
                            spacing: 10
                            Button {
                                id: bindRemoteBtn
                                text: vinManager.remoteBound ? "REBIND" : "BIND"
                                enabled: !vinManager.busy && vinManager.vin.length === 17
                                highlighted: focusIndex === 7
                                Layout.fillWidth: true
                                background: Rectangle {
                                    color: bindRemoteBtn.highlighted ? themeManager.carBlue : themeManager.bgPanel
                                    radius: 6
                                }
                                contentItem: Text {
                                    text: bindRemoteBtn.text
                                    color: themeManager.textPrimary
                                    font.pixelSize: 14
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                onClicked: vinManager.bindRemote()
                            }
                            Button {
                                id: unbindRemoteBtn
                                text: "UNBIND"
                                enabled: !vinManager.busy && vinManager.remoteBound
                                highlighted: focusIndex === 8
                                Layout.fillWidth: true
                                background: Rectangle {
                                    color: unbindRemoteBtn.highlighted ? themeManager.statusRed : themeManager.bgPanel
                                    radius: 6
                                }
                                contentItem: Text {
                                    text: unbindRemoteBtn.text
                                    color: themeManager.textPrimary
                                    font.pixelSize: 14
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                onClicked: vinManager.unbindRemote()
                            }
                        }
                    }
                }

                // ── Unbind All ──
                Rectangle {
                    Layout.fillWidth: true
                    height: 50
                    color: themeManager.bgCard
                    radius: 8
                    border.width: 1
                    border.color: Qt.rgba(themeManager.statusRed.r, themeManager.statusRed.g, themeManager.statusRed.b, 0.3)

                    Button {
                        id: unbindAllBtn
                        anchors.centerIn: parent
                        text: "UNBIND ALL MODULES"
                        enabled: !vinManager.busy && (vinManager.bodyBound || vinManager.sensorBound || vinManager.meshBound || vinManager.remoteBound)
                        highlighted: focusIndex === 9
                        width: parent.width - 20
                        background: Rectangle {
                            color: unbindAllBtn.highlighted ? themeManager.statusRed : themeManager.bgPanel
                            radius: 6
                        }
                        contentItem: Text {
                            text: unbindAllBtn.text
                            color: themeManager.textPrimary
                            font.pixelSize: 14
                            horizontalAlignment: Text.AlignHCenter
                        }
                        onClicked: vinManager.unbindAll()
                    }
                }

                // ── Secret ──
                Rectangle {
                    Layout.fillWidth: true
                    height: vinManager.secret.length > 0 ? secretCol.implicitHeight + 20 : 0
                    visible: vinManager.secret.length > 0
                    color: themeManager.bgCard
                    radius: 8
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)

                    Column {
                        id: secretCol
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 12
                        spacing: 4

                        Text {
                            text: "HMAC SECRET"
                            color: themeManager.carBlue
                            font.pixelSize: 13
                            font.bold: true
                        }
                        Text {
                            text: vinManager.secret.substring(0, 16) + "..."
                            color: themeManager.textSecondary
                            font.pixelSize: 10
                            font.family: "Monospace"
                        }
                    }
                }

                Item { Layout.fillWidth: true; height: 20 }
            }
        }
    }
}
