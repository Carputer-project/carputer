import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    focus: true
    property int activeTab: 0
    property int refreshTick: 0

    onVisibleChanged: {
        if (visible && activeTab === 0) dvrManager.startPreview()
        else if (!visible) dvrManager.stopPreview()
    }

    Timer {
        interval: 50
        running: visible && (dvrManager.previewActive || dvrManager.playing)
        repeat: true
        onTriggered: refreshTick++
    }

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
                        onClicked: {
                            activeTab = index
                            if (activeTab === 0) dvrManager.startPreview()
                        }
                    }
                }
            }
        }
    }

    Item {
        anchors {
            top: tabBar.bottom
            left: parent.left; right: parent.right; bottom: parent.bottom
        }

        // ════════════════════════════════════════════════════════════════
        // TAB 0 — RECORD with live video
        // ════════════════════════════════════════════════════════════════
        Column {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 8
            visible: activeTab === 0

            Rectangle {
                width: parent.width
                height: parent.height - 170
                color: themeManager.black
                radius: 8
                border.color: dvrManager.recording ? themeManager.statusRed : themeManager.bgPanel
                border.width: dvrManager.recording ? 3 : 1
                clip: true

                Image {
                    anchors.centerIn: parent
                    width: parent.width
                    height: parent.height
                    fillMode: Image.PreserveAspectFit
                    source: "image://video/dvr-live?" + refreshTick
                    asynchronous: true
                    cache: false
                }

                Rectangle {
                    anchors { top: parent.top; right: parent.right; margins: 8 }
                    height: 28; width: 80
                    radius: 4
                    color: dvrManager.recording ? "#CC0000" : "#333355"
                    visible: dvrManager.recording
                    Text {
                        anchors.centerIn: parent
                        text: "● REC " + dvrManager.recordingSeconds + "s"
                        color: "white"
                        font.pixelSize: 13
                        font.bold: true
                    }
                }

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

            Row {
                width: parent.width
                spacing: 10
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
                            text: dvrManager.recording ? "STOP" : "REC"
                            font.pixelSize: 28
                            font.bold: true
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
                                  ? (Math.floor(dvrManager.recordingSeconds / 60) + ":" +
                                     (dvrManager.recordingSeconds % 60 < 10 ? "0" : "") +
                                     dvrManager.recordingSeconds % 60)
                                  : "—"
                            color: dvrManager.recording ? themeManager.textPrimary : themeManager.textSecondary
                            font.pixelSize: 18; font.bold: true
                        }
                    }
                }
            }
        }

        // ════════════════════════════════════════════════════════════════
        // TAB 1 — LIBRARY with video player
        // ════════════════════════════════════════════════════════════════
        Column {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 6
            visible: activeTab === 1

            // Video player area (plays when a file is loaded)
            Rectangle {
                width: parent.width
                height: dvrManager.playingFile !== "" ? 240 : 0
                visible: height > 0
                color: themeManager.black
                radius: 8
                clip: true
                Behavior on height { NumberAnimation { duration: 180 } }

                Image {
                    anchors.centerIn: parent
                    width: parent.width
                    height: parent.height - 40
                    fillMode: Image.PreserveAspectFit
                    source: dvrManager.playing ? "image://video/dvr-playback?" + refreshTick : ""
                    asynchronous: true
                    cache: false
                }

                Text {
                    anchors.centerIn: parent
                    text: "No video playing"
                    color: themeManager.textSecondary
                    font.pixelSize: 16
                    visible: dvrManager.playingFile === ""
                }

                // Playback controls overlay at bottom of video
                Rectangle {
                    anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
                    height: 40
                    color: Qt.rgba(0, 0, 0, 0.6)
                    visible: dvrManager.playingFile !== ""

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8
                        spacing: 6

                        Text {
                            text: dvrManager.formatDuration(dvrManager.playPosition)
                            color: "white"
                            font.pixelSize: 12
                            Layout.alignment: Qt.AlignVCenter
                        }

                        Slider {
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignVCenter
                            from: 0
                            to: dvrManager.playDuration || 100
                            value: dvrManager.playPosition
                            onMoved: dvrManager.seekTo(value)
                        }

                        Text {
                            text: dvrManager.formatDuration(dvrManager.playDuration)
                            color: "#aaa"
                            font.pixelSize: 12
                            Layout.alignment: Qt.AlignVCenter
                        }

                        Rectangle {
                            width: 36; height: 28
                            radius: 4; color: themeManager.carBlueDim
                            Layout.alignment: Qt.AlignVCenter
                            Text {
                                anchors.centerIn: parent
                                text: dvrManager.playing ? "⏸" : "▶"
                                color: themeManager.carBlue
                                font.pixelSize: 16
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: dvrManager.togglePause()
                            }
                        }

                        Rectangle {
                            width: 36; height: 28
                            radius: 4; color: "#444"
                            Layout.alignment: Qt.AlignVCenter
                            Text {
                                anchors.centerIn: parent
                                text: "✕"
                                color: "white"
                                font.pixelSize: 16
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: dvrManager.stopPlayback()
                            }
                        }
                    }
                }
            }

            // File list header
            Row {
                width: parent.width
                spacing: 8
                Text {
                    text: dvrManager.recordings.length + " recording"
                          + (dvrManager.recordings.length !== 1 ? "s" : "")
                    color: themeManager.textSecondary; font.pixelSize: 14
                    anchors.verticalCenter: parent.verticalCenter
                }
                Item { Layout.fillWidth: true; width: 1; height: 1 }
                Rectangle {
                    width: 90; height: 34; radius: 6; color: themeManager.bgCard
                    Text { anchors.centerIn: parent; text: "↻ Refresh"; color: themeManager.carBlue; font.pixelSize: 13 }
                    MouseArea { anchors.fill: parent; onClicked: dvrManager.scanRecordings() }
                }
            }

            // File list
            Rectangle {
                width: parent.width
                height: parent.height - (dvrManager.playingFile !== "" ? 270 : 0) - 42 - 16
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
                                    text: modelData.split("/").pop().replace("dashcam_", "").replace(".mkv","").replace("_"," ")
                                    color: themeManager.textSecondary; font.pixelSize: 11
                                }
                            }
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
        }
    }

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
}
