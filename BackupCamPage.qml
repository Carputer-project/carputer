import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    focus: true
    property int activeTab: 0
    property int refreshTick: 0

    onVisibleChanged: {
        if (visible && activeTab === 0 && cameraManager.streaming)
            cameraManager.startStream()
        else if (!visible && cameraManager.streaming)
            cameraManager.stopStream()
    }

    Timer {
        interval: 50
        running: visible && (cameraManager.streaming || dvrManager.playing)
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
                model: ["📷 CAMERA", "📂 LIBRARY"]
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
                            if (activeTab === 0 && !cameraManager.streaming)
                                cameraManager.startStream()
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
        // TAB 0 — CAMERA live view
        // ════════════════════════════════════════════════════════════════
        ColumnLayout {
            anchors.fill: parent
            spacing: 0
            visible: activeTab === 0

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: themeManager.black

                Image {
                    anchors.centerIn: parent
                    width: parent.width
                    height: parent.height
                    fillMode: Image.PreserveAspectFit
                    source: cameraManager.streaming ? "image://video/camera?" + refreshTick : ""
                    asynchronous: true
                    cache: false
                }

                Text {
                    anchors.centerIn: parent
                    text: !cameraManager.streaming ? "TAP TO START CAMERA" : ""
                    color: themeManager.textSecondary
                    font.pixelSize: 32
                    visible: !cameraManager.streaming
                }

                Rectangle {
                    anchors { left: parent.left; bottom: parent.bottom; margins: 10 }
                    width: 200
                    height: 30
                    color: cameraManager.streaming ? themeManager.carOrange : "#444"
                    Text {
                        anchors.centerIn: parent
                        text: cameraManager.streaming ? cameraManager.device : "Offline"
                        color: themeManager.black
                        font.pixelSize: 14
                        font.bold: true
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if (cameraManager.streaming) cameraManager.stopStream()
                        else cameraManager.startStream()
                    }
                }
            }

            // Control bar
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 80
                color: themeManager.bgCard
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10
                    Rectangle {
                        Layout.preferredWidth: 120
                        Layout.fillHeight: true
                        radius: 8
                        color: cameraManager.streaming ? "#330000" : themeManager.bgDark
                        border.color: cameraManager.streaming ? themeManager.statusRed : themeManager.carBlue
                        border.width: 2
                        Text {
                            anchors.centerIn: parent
                            text: cameraManager.streaming ? "STOP" : "START"
                            color: cameraManager.streaming ? themeManager.statusRed : themeManager.carBlue
                            font.pixelSize: 16
                            font.bold: true
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                if (cameraManager.streaming) cameraManager.stopStream()
                                else cameraManager.startStream()
                            }
                        }
                    }
                    Rectangle {
                        Layout.preferredWidth: 100
                        Layout.fillHeight: true
                        radius: 8
                        color: themeManager.bgDark
                        border.color: themeManager.bgPanel
                        border.width: 1
                        Text {
                            anchors.centerIn: parent
                            text: cameraManager.device.split("/").pop()
                            color: themeManager.textPrimary
                            font.pixelSize: 14
                            font.bold: true
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: cameraPicker.open()
                        }
                    }
                    Item { Layout.fillWidth: true }
                    Rectangle {
                        Layout.preferredWidth: 100
                        Layout.fillHeight: true
                        radius: 8
                        color: themeManager.bgDark
                        border.color: themeManager.bgPanel
                        border.width: 1
                        Text {
                            anchors.centerIn: parent
                            text: "DASHCA"
                            color: themeManager.carBlue
                            font.pixelSize: 14
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: activeTab = 1
                        }
                    }
                }
            }
        }

        // ════════════════════════════════════════════════════════════════
        // TAB 1 — DVR LIBRARY with video player
        // ════════════════════════════════════════════════════════════════
        Column {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 6
            visible: activeTab === 1

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

            Row {
                width: parent.width
                spacing: 8
                Text {
                    text: dvrManager.recordings.length + " recording"
                          + (dvrManager.recordings.length !== 1 ? "s" : "")
                    color: themeManager.textSecondary; font.pixelSize: 14
                    anchors.verticalCenter: parent.verticalCenter
                }
                Item { width: 1; height: 1; Layout.fillWidth: true }
                Rectangle {
                    width: 90; height: 34; radius: 6; color: themeManager.bgCard
                    Text { anchors.centerIn: parent; text: "↻ Refresh"; color: themeManager.carBlue; font.pixelSize: 13 }
                    MouseArea { anchors.fill: parent; onClicked: dvrManager.scanRecordings() }
                }
            }

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
                                    onClicked: deleteCamFile.filePath = modelData
                                }
                            }
                        }
                    }
                    Text {
                        anchors.centerIn: parent
                        visible: dvrManager.recordings.length === 0
                        text: "No DVR recordings yet."
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
                model: ["/dev/video0", "/dev/video1", "/dev/video2"]
                Rectangle {
                    width: parent.width; height: 46; radius: 6
                    color: cameraManager.device === modelData ? themeManager.carBlueDim : themeManager.bgCard
                    Text {
                        anchors.centerIn: parent
                        text: modelData
                        color: cameraManager.device === modelData ? themeManager.carBlue : themeManager.textPrimary
                        font.pixelSize: 14
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            cameraManager.setDevice(modelData)
                            if (cameraManager.streaming) cameraManager.startStream()
                            cameraPicker.close()
                        }
                    }
                }
            }
        }
    }

    QtObject {
        id: deleteCamFile
        property string filePath: ""
        onFilePathChanged: if (filePath !== "") confirmCamDialog.open()
    }
    Dialog {
        id: confirmCamDialog
        title: "Delete Recording?"
        anchors.centerIn: parent
        width: 320
        Column {
            width: parent.width
            spacing: 16
            Text {
                width: parent.width
                text: deleteCamFile.filePath.split("/").pop()
                color: themeManager.textPrimary; font.pixelSize: 14
                wrapMode: Text.WrapAtWordBoundaryOrAnywhere
            }
            Row {
                width: parent.width
                spacing: 10
                Rectangle {
                    width: (parent.width - 10) / 2; height: 44; radius: 8; color: themeManager.bgCard
                    Text { anchors.centerIn: parent; text: "Cancel"; color: themeManager.textPrimary; font.pixelSize: 15 }
                    MouseArea { anchors.fill: parent; onClicked: { confirmCamDialog.close(); deleteCamFile.filePath = "" } }
                }
                Rectangle {
                    width: (parent.width - 10) / 2; height: 44; radius: 8; color: "#330000"
                    border.color: themeManager.statusRed; border.width: 1
                    Text { anchors.centerIn: parent; text: "Delete"; color: themeManager.statusRed; font.pixelSize: 15; font.bold: true }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            dvrManager.deleteFile(deleteCamFile.filePath)
                            confirmCamDialog.close()
                            deleteCamFile.filePath = ""
                        }
                    }
                }
            }
        }
    }
}
