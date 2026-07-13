import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: tripLogPage
    anchors.fill: parent
    focus: true

    property var trips: []
    property int selectedTrip: -1
    property var summary: ({})

    Component.onCompleted: refreshTrips()

    function refreshTrips() {
        trips = dataLogger.listTrips()
        selectedTrip = -1
        summary =({})
    }

    function loadTrip(index) {
        selectedTrip = index
        if (index >= 0 && index < trips.length) {
            summary = dataLogger.tripSummary(trips[index].path)
        }
    }

    function formatDuration(rows) {
        var secs = rows * 2
        var h = Math.floor(secs / 3600)
        var m = Math.floor((secs % 3600) / 60)
        var s = secs % 60
        if (h > 0) return h + "h " + m + "m"
        if (m > 0) return m + "m " + s + "s"
        return s + "s"
    }

    function formatTimestamp(ts) {
        if (!ts || ts.length < 16) return ts || "---"
        return ts.substring(0, 10) + "  " + ts.substring(11, 16)
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_Escape || event.key === Qt.Key_Backspace) {
            if (selectedTrip >= 0) { selectedTrip = -1; summary =({}); event.accepted = true; }
        }
        if (event.key === Qt.Key_Up || event.key === Qt.Key_Down) {
            var dir = event.key === Qt.Key_Up ? -1 : 1
            var newIdx = selectedTrip + dir
            if (selectedTrip < 0) newIdx = dir > 0 ? 0 : trips.length - 1
            if (newIdx >= 0 && newIdx < trips.length) loadTrip(newIdx)
            event.accepted = true
        }
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Space) {
            if (selectedTrip >= 0) { selectedTrip = -1; summary =({}); }
            else if (trips.length > 0) loadTrip(0)
            event.accepted = true
        }
    }

    Rectangle {
        anchors.fill: parent
        color: themeManager.bgDark

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            // Header
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
                Row {
                    anchors.centerIn: parent
                    spacing: 12
                    Text {
                        text: "TRIP LOG"
                        color: themeManager.carBlue
                        font.pixelSize: 24
                        font.bold: true
                    }
                    Text {
                        anchors.baseline: parent.baseline
                        text: trips.length + " trips"
                        color: themeManager.textSecondary
                        font.pixelSize: 14
                    }
                }
            }

            // Trip list
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: selectedTrip < 0
                Layout.preferredHeight: selectedTrip >= 0 ? parent.height * 0.4 : -1
                visible: selectedTrip < 0 || true
                color: themeManager.bgCard
                radius: 8
                border.width: 1
                border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)

                ListView {
                    id: tripList
                    anchors.fill: parent
                    anchors.margins: 8
                    clip: true
                    model: trips
                    highlightFollowsCurrentItem: true
                    currentIndex: selectedTrip

                    header: Rectangle {
                        width: tripList.width
                        height: 32
                        color: "transparent"
                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            Text { text: "DATE"; color: themeManager.textSecondary; font.pixelSize: 11; font.bold: true; Layout.fillWidth: true }
                            Text { text: "DURATION"; color: themeManager.textSecondary; font.pixelSize: 11; font.bold: true; Layout.preferredWidth: 120 }
                            Text { text: "ROWS"; color: themeManager.textSecondary; font.pixelSize: 11; font.bold: true; Layout.preferredWidth: 80 }
                        }
                    }

                    delegate: Rectangle {
                        width: tripList.width
                        height: 44
                        color: tripList.currentIndex === index ? Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.20) : themeManager.bgCard
                        radius: 6

                        MouseArea {
                            anchors.fill: parent
                            onClicked: loadTrip(index)
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 8
                            Text {
                                text: formatTimestamp(modelData.firstTimestamp)
                                color: themeManager.textPrimary
                                font.pixelSize: 14
                                Layout.fillWidth: true
                            }
                            Text {
                                text: formatDuration(modelData.rows)
                                color: themeManager.textSecondary
                                font.pixelSize: 13
                                Layout.preferredWidth: 120
                            }
                            Text {
                                text: modelData.rows
                                color: themeManager.textSecondary
                                font.pixelSize: 13
                                Layout.preferredWidth: 80
                                horizontalAlignment: Text.AlignRight
                            }
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        visible: trips.length === 0
                        text: "No trips logged yet.\nTrips are recorded automatically when the app runs."
                        color: themeManager.textSecondary
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }

            // Detail view
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: selectedTrip >= 0
                color: themeManager.bgCard
                radius: 8
                border.width: 1
                border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)

                Flickable {
                    anchors.fill: parent
                    anchors.margins: 12
                    contentHeight: detailColumn.implicitHeight
                    clip: true

                    ColumnLayout {
                        id: detailColumn
                        width: parent.width
                        spacing: 10

                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: formatTimestamp(summary.firstTimestamp || "")
                                color: themeManager.textPrimary
                                font.pixelSize: 16
                                font.bold: true
                                Layout.fillWidth: true
                            }
                            Rectangle {
                                Layout.preferredWidth: backLabel.implicitWidth + 24
                                Layout.preferredHeight: 32
                                color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.15)
                                radius: 6
                                border.width: 1
                                border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.35)
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: { selectedTrip = -1; summary =({}); }
                                }
                                Text {
                                    id: backLabel
                                    anchors.centerIn: parent
                                    text: "← BACK"
                                    color: themeManager.carBlue
                                    font.pixelSize: 12
                                    font.bold: true
                                }
                            }
                        }

                        Text {
                            text: "Duration: " + formatDuration(summary.rows || 0) + "  ·  " + (summary.rows || 0) + " samples"
                            color: themeManager.textSecondary
                            font.pixelSize: 13
                        }

                        // Key stats grid
                        GridLayout {
                            Layout.fillWidth: true
                            columns: 3
                            columnSpacing: 10
                            rowSpacing: 6

                            Repeater {
                                model: [
                                    { label: "AVG SPEED", value: (summary.avgSpeed || 0).toFixed(1), unit: "km/h", color: themeManager.textPrimary },
                                    { label: "MAX SPEED", value: (summary.maxSpeed || 0).toFixed(1), unit: "km/h", color: themeManager.textPrimary },
                                    { label: "AVG RPM", value: Math.round(summary.avgRpm || 0), unit: "rpm", color: themeManager.textPrimary },
                                    { label: "MAX RPM", value: Math.round(summary.maxRpm || 0), unit: "rpm", color: (summary.maxRpm || 0) > 5500 ? themeManager.warnColor : themeManager.textPrimary },
                                    { label: "MAX COOLANT", value: (summary.maxCoolant || 0).toFixed(1), unit: "°C", color: (summary.maxCoolant || 0) > 100 ? themeManager.warnColor : themeManager.textPrimary },
                                    { label: "MAX OIL", value: (summary.maxOil || 0).toFixed(1), unit: "°C", color: (summary.maxOil || 0) > 120 ? themeManager.warnColor : themeManager.textPrimary },
                                    { label: "MIN BATTERY", value: (summary.minBattery || 0).toFixed(1), unit: "V", color: (summary.minBattery || 0) < 12 ? themeManager.warnColor : themeManager.textPrimary },
                                    { label: "AVG BATTERY", value: (summary.avgBattery || 0).toFixed(1), unit: "V", color: themeManager.textPrimary },
                                    { label: "AVG AFR", value: (summary.avgO2AFR || 0).toFixed(2), unit: "", color: (summary.avgO2AFR || 0) > 0 ? themeManager.carBlue : themeManager.textSecondary }
                                ]
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    Text {
                                        text: modelData.label
                                        color: themeManager.textSecondary
                                        font.pixelSize: 10
                                        font.bold: true
                                    }
                                    Text {
                                        text: modelData.value + " " + modelData.unit
                                        color: modelData.color
                                        font.pixelSize: 20
                                        font.bold: true
                                    }
                                }
                            }
                        }

                        // Time range
                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.15)
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            Text {
                                text: "From: " + formatTimestamp(summary.firstTimestamp || "")
                                color: themeManager.textSecondary
                                font.pixelSize: 12
                                Layout.fillWidth: true
                            }
                            Text {
                                text: "To: " + formatTimestamp(summary.lastTimestamp || "")
                                color: themeManager.textSecondary
                                font.pixelSize: 12
                            }
                        }
                    }
                }
            }
        }
    }
}
