import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: alarmPage
    focus: true

    property int focusIndex: 0
    property int focusCount: 4

    function navigateUp() { focusIndex = Math.max(0, focusIndex - 1); }
    function navigateDown() { focusIndex = Math.min(focusCount - 1, focusIndex + 1); }
    function activateCurrentFocus() {
        if (focusIndex === 0) armBtn.clicked();
        else if (focusIndex === 1) disarmBtn.clicked();
        else if (focusIndex === 2) clearBtn.clicked();
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
                        text: "SECURITY ALARM"
                        color: themeManager.carBlue
                        font.pixelSize: 24
                        font.bold: true
                    }
                }

                // ── Status ──
                Rectangle {
                    Layout.fillWidth: true
                    height: 44
                    color: themeManager.bgCard
                    radius: 8
                    border.width: 2
                    border.color: alarmManager.triggered ? themeManager.statusRed
                                    : alarmManager.armed ? themeManager.statusGreen
                                    : Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)

                    Row {
                        anchors.centerIn: parent
                        spacing: 12
                        Text {
                            text: "STATE:"
                            color: themeManager.textSecondary
                            font.pixelSize: 14
                            font.bold: true
                        }
                        Text {
                            text: alarmManager.alarmState.toUpperCase()
                            color: alarmManager.triggered ? themeManager.statusRed
                                    : alarmManager.armed ? themeManager.statusGreen
                                    : themeManager.textPrimary
                            font.pixelSize: 18
                            font.bold: true
                        }
                        Text {
                            text: alarmManager.sirenOn ? " ◉ SIREN" : ""
                            color: themeManager.statusRed
                            font.pixelSize: 14
                            font.bold: true
                            visible: alarmManager.sirenOn
                        }
                    }
                }

                // ── Status message ──
                Rectangle {
                    Layout.fillWidth: true
                    height: alarmManager.statusText.length > 0 ? 32 : 0
                    visible: alarmManager.statusText.length > 0
                    color: themeManager.bgCard
                    radius: 6
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)
                    Text {
                        anchors.centerIn: parent
                        text: alarmManager.statusText
                        color: alarmManager.busy ? themeManager.carOrange : themeManager.statusGreen
                        font.pixelSize: 14
                        font.bold: true
                    }
                }

                // ── Arm/Disarm buttons ──
                Rectangle {
                    Layout.fillWidth: true
                    height: 60
                    color: themeManager.bgCard
                    radius: 8
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        Button {
                            id: armBtn
                            text: "ARM"
                            enabled: !alarmManager.busy && !alarmManager.armed
                            highlighted: focusIndex === 0
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            background: Rectangle {
                                color: armBtn.highlighted ? themeManager.statusGreen
                                        : armBtn.enabled ? Qt.rgba(themeManager.statusGreen.r, themeManager.statusGreen.g, themeManager.statusGreen.b, 0.2)
                                        : themeManager.bgPanel
                                radius: 6
                            }
                            contentItem: Text {
                                text: armBtn.text
                                color: armBtn.enabled ? themeManager.statusGreen : themeManager.textSecondary
                                font.pixelSize: 18
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: alarmManager.arm()
                        }

                        Button {
                            id: disarmBtn
                            text: "DISARM"
                            enabled: !alarmManager.busy && (alarmManager.armed || alarmManager.triggered)
                            highlighted: focusIndex === 1
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            background: Rectangle {
                                color: disarmBtn.highlighted ? themeManager.statusRed
                                        : disarmBtn.enabled ? Qt.rgba(themeManager.statusRed.r, themeManager.statusRed.g, themeManager.statusRed.b, 0.2)
                                        : themeManager.bgPanel
                                radius: 6
                            }
                            contentItem: Text {
                                text: disarmBtn.text
                                color: disarmBtn.enabled ? themeManager.statusRed : themeManager.textSecondary
                                font.pixelSize: 18
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: alarmManager.disarm()
                        }
                    }
                }

                // ── Event Log ──
                Rectangle {
                    Layout.fillWidth: true
                    height: eventLogColumn.implicitHeight + 20
                    color: themeManager.bgCard
                    radius: 8
                    border.width: 1
                    border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)

                    Column {
                        id: eventLogColumn
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.margins: 12
                        spacing: 8

                        RowLayout {
                            width: parent.width
                            Text {
                                text: "EVENT LOG (" + alarmManager.eventCount + ")"
                                color: themeManager.carBlue
                                font.pixelSize: 13
                                font.bold: true
                                Layout.fillWidth: true
                            }
                            Button {
                                id: clearBtn
                                text: "CLEAR"
                                enabled: alarmManager.eventCount > 0
                                highlighted: focusIndex === 2
                                background: Rectangle {
                                    color: clearBtn.highlighted ? themeManager.statusRed : themeManager.bgPanel
                                    radius: 6
                                }
                                contentItem: Text {
                                    text: clearBtn.text
                                    color: themeManager.textPrimary
                                    font.pixelSize: 12
                                    horizontalAlignment: Text.AlignHCenter
                                }
                                onClicked: alarmManager.clearEvents()
                            }
                        }

                        Text {
                            text: "No events recorded"
                            color: themeManager.textSecondary
                            font.pixelSize: 12
                            visible: alarmManager.eventCount === 0
                        }

                        Repeater {
                            model: Math.min(alarmManager.eventCount, 20)
                            Rectangle {
                                Layout.fillWidth: true
                                height: 32
                                color: index % 2 === 0 ? Qt.rgba(0, 0, 0, 0.1) : "transparent"
                                radius: 4

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 4
                                    spacing: 8
                                    Text {
                                        text: alarmManager.eventTimestamp(index)
                                        color: themeManager.textSecondary
                                        font.pixelSize: 10
                                        font.family: "Monospace"
                                        Layout.preferredWidth: 140
                                    }
                                    Text {
                                        text: alarmManager.eventTrigger(index)
                                        color: {
                                            var sev = alarmManager.eventSeverity(index);
                                            if (sev === "critical") return themeManager.statusRed;
                                            if (sev === "high") return themeManager.carOrange;
                                            return themeManager.textPrimary;
                                        }
                                        font.pixelSize: 12
                                        font.bold: true
                                        Layout.fillWidth: true
                                    }
                                    Text {
                                        text: alarmManager.eventSource(index)
                                        color: themeManager.textSecondary
                                        font.pixelSize: 11
                                    }
                                }
                            }
                        }
                    }
                }

                Item { Layout.fillWidth: true; height: 20 }
            }
        }
    }
}
