import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
Item {
    focus: true
    // ── Internal state ────────────────────────────────────────────────────────
    property int activeTab: 0   // 0 = Record, 1 = Library
    // ── Tab bar ───────────────────────────────────────────────────────────────
    Rectangle {
        id: tabBar
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: 50
        color: themeManager.bgPanel
        Row {
            anchors.fill: parent
            spacing: 0
            Repeater {
                model: ["● RECORD", "📂 LIBRARY"]
                Rectangle {
                    width: tabBar.width / 2
                    height: tabBar.height
                    color: activeTab === index ? themeManager.bgCard : "transparent"
                    Rectangle {
                        anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
                        height: 3
                        color: activeTab === index ? themeManager.carBlue : "transparent"
                    }
                    Text {
                        anchors.centerIn: parent
                        text: modelData
                        color: activeTab === index ? themeManager.carBlue : themeManager.textSecondary
                        font.pixelSize: 16
                        font.bold: activeTab === index
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: activeTab = index
                    }
                }
            }
        }
    }
    // ── Content area (below tab bar) ──────────────────────────────────────────
    Item {
        anchors {
            top: tabBar.bottom
            left: parent.left; right: parent.right; bottom: parent.bottom
        }
        // ════════════════════════════════════════════════════════════════════
        // TAB 0 — RECORD
        // ════════════════════════════════════════════════════════════════════
        Column {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 8
            visible: activeTab === 0
            // Camera preview area
            Rectangle {
                width: parent.width
                height: parent.height - 170
                color: themeManager.bgDark
                radius: 8
                border.color: dvrManager.recording ? themeManager.statusRed : themeManager.bgPanel
                border.width: dvrManager.recording ? 3 : 1
                Rectangle {
                    anchors.centerIn: parent
                    width: 80; height: 80
                    radius: 40
                    color: "transparent"
                    border.color: themeManager.statusRed
                    border.width: 3
                    visible: dvrManager.recording
                    NumberAnimation on opacity {
                        id: recordPulse
                        running: dvrManager.recording
                        from: 1.0; to: 0.1
                        duration: 900
                        loops: Animation.Infinite
                    }
                }
                Column {
                    anchors.centerIn: parent
                    spacing: 8
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: dvrManager.recording ? "● REC" : "○"
                        color: dvrManager.recording ? themeManager.statusRed : "#333355"
                        font.pixelSize: dvrManager.recording ? 28 : 64
                        font.bold: true
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: dvrManager.recording
                              ? formatRecTime(dvrManager.recordingSeconds)
                              : "Camera: " + dvrManager.cameraSource.split("/").pop()
                        color: dvrManager.recording ? themeManager.textPrimary : "#333355"
                        font.pixelSize: dvrManager.recording ? 36 : 18
                        font.bold: dvrManager.recording
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        visible: dvrManager.recording
                        text: dvrManager.currentFile.split("/").pop()
                        color: themeManager.textSecondary
                        font.pixelSize: 11
                        elide: Text.ElideLeft
                        width: 300
                    }
                }
                // Camera source chip
                Rectangle {
                    anchors { bottom: parent.bottom; left: parent.left; margins: 10 }
                    height: 26; width: sourceLabel.implicitWidth + 20
                    radius: 4
                    color: themeManager.bgPanel
                    Text {
                        id: sourceLabel
                        anchors.centerIn: parent
                        text: dvrManager.cameraSource
                        color: themeManager.textSecondary
                        font.pixelSize: 11
                    }
                }
            }
            // ── Transport controls ────────────────────────────────────────
            Row {
                width: parent.width
                spacing: 10
                // Big RECORD / STOP button
                Rectangle {
                    width: (parent.width - 10) * 0.65
                    height: 72
                    radius: 10
                    color: dvrManager.recording ? "#330000" : themeManager.bgDark
                    border.color: dvrManager.recording ? themeManager.statusRed : themeManager.carBlue
                    border.width: 2
                    Row {
                        anchors.centerIn: parent
                        spacing: 12
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: dvrManager.recording ? "⏹" : "⏺"
                            font.pixelSize: 30
                            color: dvrManager.recording ? themeManager.statusRed : themeManager.carBlue
                        }
                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            text: dvrManager.recording ? "STOP RECORDING" : "START RECORDING"
                            color: dvrManager.recording ? themeManager.statusRed : themeManager.carBlue
                            font.pixelSize: 18
                            font.bold: true
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: dvrManager.recording ? dvrManager.stopRecording()
                                                        : dvrManager.startRecording()
                    }
                }
                // Camera source selector
                Rectangle {
                    width: (parent.width - 10) * 0.35
                    height: 72
                    radius: 10
                    color: themeManager.bgCard
                    border.color: themeManager.bgPanel
                    border.width: 1
                    Column {
                        anchors.centerIn: parent
                        spacing: 4
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "CAMERA"
                            color: themeManager.textSecondary
                            font.pixelSize: 10
                        }
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: dvrManager.cameraSource.split("/").pop()
                            color: themeManager.textPrimary
                            font.pixelSize: 15
                            font.bold: true
                        }
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "tap to change"
                            color: themeManager.textSecondary
                            font.pixelSize: 10
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: cameraPicker.open()
                    }
                }
            }
            // ── Quick stats row ───────────────────────────────────────────
            Row {
                width: parent.width
                spacing: 8
                Rectangle {
                    width: (parent.width - 16) / 3
                    height: 52
                    radius: 6; color: themeManager.bgCard
                    Column {
                        anchors.centerIn: parent
                        spacing: 2
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "FILES"
                            color: themeManager.textSecondary; font.pixelSize: 10
                        }
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: dvrManager.recordings.length
                            color: themeManager.carBlue; font.pixelSize: 22; font.bold: true
                        }
                    }
                }
                Rectangle {
                    width: (parent.width - 16) / 3
                    height: 52
                    radius: 6; color: themeManager.bgCard
                    Column {
                        anchors.centerIn: parent
                        spacing: 2
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "SAVE TO"
                            color: themeManager.textSecondary; font.pixelSize: 10
                        }
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: dvrManager.dvrDirectory.split("/").pop() || "dvr"
                            color: themeManager.textPrimary; font.pixelSize: 15; font.bold: true
                        }
                    }
                }
                Rectangle {
                    width: (parent.width - 16) / 3
                    height: 52
                    radius: 6; color: themeManager.bgCard
                    Column {
                        anchors.centerIn: parent
                        spacing: 2
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: dvrManager.recording ? "RECORDING" : "READY"
                            color: dvrManager.recording ? themeManager.statusRed : themeManager.textSecondary
                            font.pixelSize: 10
                        }
                        Text {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: dvrManager.recording
                                  ? formatRecTime(dvrManager.recordingSeconds)
                                  : "—"
                            color: dvrManager.recording ? themeManager.textPrimary : themeManager.textSecondary
                            font.pixelSize: 18; font.bold: true
                        }
                    }
                }
            }
        } // end Record tab column
        // ════════════════════════════════════════════════════════════════════
        // TAB 1 — LIBRARY
        // ════════════════════════════════════════════════════════════════════
        Column {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 8
            visible: activeTab === 1
            // ── Mini player (shown when a file is loaded) ─────────────────
            Rectangle {
                width: parent.width
                height: dvrManager.playingFile !== "" ? 110 : 0
                visible: height > 0
                color: themeManager.bgCard
                radius: 8
                clip: true
                Behavior on height { NumberAnimation { duration: 180 } }
                Column {
                    anchors { fill: parent; margins: 12 }
                    spacing: 6
                    // File name + stop button
                    Row {
                        width: parent.width
                        spacing: 8
                        Text {
                            width: parent.width - 44
                            text: dvrManager.playingFile.split("/").pop()
                            color: themeManager.textPrimary
                            font.pixelSize: 14
                            font.bold: true
                            elide: Text.ElideLeft
                        }
                        Rectangle {
                            width: 36; height: 36
                            radius: 6; color: themeManager.bgPanel
                            Text { anchors.centerIn: parent; text: "✕"; color: themeManager.textSecondary; font.pixelSize: 16 }
                            MouseArea { anchors.fill: parent; onClicked: dvrManager.stopPlayback() }
                        }
                    }
                    // Seek slider
                    Slider {
                        width: parent.width
                        from: 0
                        to: dvrManager.playDuration || 100
                        value: dvrManager.playPosition
                        onMoved: dvrManager.seekTo(value)
                    }
                    // Time + play/pause
                    RowLayout {
                        width: parent.width
                        spacing: 8
                        Text {
                            text: dvrManager.formatDuration(dvrManager.playPosition)
                                  + " / "
                                  + dvrManager.formatDuration(dvrManager.playDuration)
                            color: themeManager.textSecondary; font.pixelSize: 12
                            Layout.alignment: Qt.AlignVCenter
                        }
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            width: 48; height: 32
                            radius: 6; color: themeManager.carBlueDim
                            Layout.alignment: Qt.AlignVCenter
                            Text {
                                anchors.centerIn: parent
                                text: dvrManager.playing ? "⏸" : "▶"
                                color: themeManager.carBlue; font.pixelSize: 18
                            }
                            MouseArea { anchors.fill: parent; onClicked: dvrManager.togglePause() }
                        }
                    }
                }
            }
            // ── Header row ────────────────────────────────────────────────
            Row {
                width: parent.width
                spacing: 8
                Text {
                    text: dvrManager.recordings.length + " recording"
                          + (dvrManager.recordings.length !== 1 ? "s" : "")
                    color: themeManager.textSecondary; font.pixelSize: 14
                    anchors.verticalCenter: parent.verticalCenter
                }
                Item { width: 1; height: 1; implicitWidth: parent.width - 200 }
                Rectangle {
                    width: 90; height: 34; radius: 6; color: themeManager.bgCard
                    Text { anchors.centerIn: parent; text: "↻ Refresh"; color: themeManager.carBlue; font.pixelSize: 13 }
                    MouseArea { anchors.fill: parent; onClicked: dvrManager.scanRecordings() }
                }
            }
            // ── File list ─────────────────────────────────────────────────
            Rectangle {
                width: parent.width
                height: parent.height - (dvrManager.playingFile !== "" ? 110 : 0) - 42 - 16
                color: "transparent"
                clip: true
                ListView {
                    anchors.fill: parent
                    model: dvrManager.recordings
                    spacing: 6
                    clip: true
                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 68
                        radius: 8
                        color: dvrManager.playingFile === modelData ? themeManager.carBlueDim : themeManager.bgCard
                        border.color: dvrManager.playingFile === modelData ? themeManager.carBlue : "transparent"
                        border.width: 1
                        Row {
                            anchors { fill: parent; leftMargin: 14; rightMargin: 10 }
                            spacing: 10
                            // File info
                            Column {
                                width: parent.width - 120
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 3
                                Text {
                                    text: dvrManager.fileLabel(modelData)
                                    color: themeManager.textPrimary; font.pixelSize: 14; font.bold: true
                                    elide: Text.ElideLeft; width: parent.width
                                }
                                Text {
                                    text: fileSizeLabel(modelData)
                                    color: themeManager.textSecondary; font.pixelSize: 11
                                }
                            }
                            // Play button
                            Rectangle {
                                width: 48; height: 48
                                radius: 8; color: themeManager.bgPanel
                                anchors.verticalCenter: parent.verticalCenter
                                Text { anchors.centerIn: parent; text: "▶"; color: themeManager.carBlue; font.pixelSize: 20 }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: {
                                        dvrManager.playFile(modelData)
                                        activeTab = 1
                                    }
                                }
                            }
                            // Delete button
                            Rectangle {
                                width: 48; height: 48
                                radius: 8; color: themeManager.bgPanel
                                anchors.verticalCenter: parent.verticalCenter
                                Text { anchors.centerIn: parent; text: "🗑"; font.pixelSize: 20 }
                                MouseArea {
                                    anchors.fill: parent
                                    onClicked: deleteConfirm.filePath = modelData
                                }
                            }
                        }
                    }
                    // Empty state
                    Text {
                        anchors.centerIn: parent
                        visible: dvrManager.recordings.length === 0
                        text: "No recordings yet.\nSwitch to Record tab to start."
                        color: themeManager.textSecondary; font.pixelSize: 16
                        horizontalAlignment: Text.AlignHCenter
                        lineHeight: 1.5
                    }
                }
            }
        } // end Library tab column
    } // end content area
    // ── Camera picker dialog ──────────────────────────────────────────────────
    Dialog {
        id: cameraPicker
        title: "Select Camera Source"
        anchors.centerIn: parent
        width: 320
        Column {
            width: parent.width
            spacing: 8
            Repeater {
                model: ["/dev/video0", "/dev/video1", "/dev/video2", "udp://0.0.0.0:5600"]
                Rectangle {
                    width: parent.width; height: 46; radius: 6
                    color: dvrManager.cameraSource === modelData ? themeManager.carBlueDim : themeManager.bgCard
                    Text {
                        anchors.centerIn: parent
                        text: modelData
                        color: dvrManager.cameraSource === modelData ? themeManager.carBlue : themeManager.textPrimary
                        font.pixelSize: 14
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: { dvrManager.setCameraSource(modelData); cameraPicker.close() }
                    }
                }
            }
        }
    }
    // ── Delete confirmation ───────────────────────────────────────────────────
    QtObject {
        id: deleteConfirm
        property string filePath: ""
        onFilePathChanged: if (filePath !== "") confirmDialog.open()
    }
    Dialog {
        id: confirmDialog
        title: "Delete Recording?"
        anchors.centerIn: parent
        width: 320
        Column {
            width: parent.width
            spacing: 16
            Text {
                width: parent.width
                text: deleteConfirm.filePath.split("/").pop()
                color: themeManager.textPrimary; font.pixelSize: 14
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }
            Row {
                width: parent.width
                spacing: 10
                Rectangle {
                    width: (parent.width - 10) / 2; height: 44; radius: 8; color: themeManager.bgCard
                    Text { anchors.centerIn: parent; text: "Cancel"; color: themeManager.textPrimary; font.pixelSize: 15 }
                    MouseArea { anchors.fill: parent; onClicked: { confirmDialog.close(); deleteConfirm.filePath = "" } }
                }
                Rectangle {
                    width: (parent.width - 10) / 2; height: 44; radius: 8; color: "#330000"
                    border.color: themeManager.statusRed; border.width: 1
                    Text { anchors.centerIn: parent; text: "Delete"; color: themeManager.statusRed; font.pixelSize: 15; font.bold: true }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            dvrManager.deleteFile(deleteConfirm.filePath)
                            confirmDialog.close()
                            deleteConfirm.filePath = ""
                        }
                    }
                }
            }
        }
    }
    // ── Helpers ───────────────────────────────────────────────────────────────
    function formatRecTime(seconds) {
        var h = Math.floor(seconds / 3600)
        var m = Math.floor((seconds % 3600) / 60)
        var s = seconds % 60
        if (h > 0)
            return pad(h) + ":" + pad(m) + ":" + pad(s)
        return pad(m) + ":" + pad(s)
    }
    function pad(n) { return n < 10 ? "0" + n : "" + n }
    function fileSizeLabel(path) {
        // QML can't stat files, just show a placeholder
        return path.split("/").pop().replace("dashcam_", "").replace(".mkv","").replace("_"," ")
    }
}
