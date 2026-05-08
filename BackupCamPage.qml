import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
Item {
    focus: true
    ColumnLayout {
        anchors.fill: parent
        spacing: 0
        // Camera view (70% of screen)
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: themeManager.black
            Text {
                anchors.centerIn: parent
                text: cameraManager.streaming ? "" : "TAP TO START CAMERA"
                color: themeManager.textSecondary
                font.pixelSize: 32
                visible: !cameraManager.streaming
            }
            Text {
                anchors.centerIn: parent
                text: "Camera: " + cameraManager.device
                color: themeManager.textSecondary
                font.pixelSize: 18
                visible: cameraManager.streaming
            }
            Rectangle {
                anchors {
                    left: parent.left
                    bottom: parent.bottom
                    margins: 10
                }
                width: 200
                height: 30
                color: themeManager.carOrange
                Text {
                    anchors.centerIn: parent
                    text: cameraManager.device
                    color: themeManager.black
                    font.pixelSize: 14
                    font.bold: true
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (cameraManager.streaming) cameraManager.stopStream();
                    else cameraManager.startStream();
                }
            }
        }
        // Control bar (30% of screen)
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: themeManager.bgCard
            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10
                Button {
                    Layout.preferredWidth: 100
                    Layout.fillHeight: true
                    text: cameraManager.streaming ? "STOP" : "START"
                    font.pixelSize: 14
                    font.bold: true
                    onClicked: {
                        if (cameraManager.streaming) cameraManager.stopStream();
                        else cameraManager.startStream();
                    }
                }
                Button {
                    Layout.preferredWidth: 100
                    Layout.fillHeight: true
                    text: "SETTINGS"
                    font.pixelSize: 14
                    onClicked: root.activePage = 7;
                }
                Item { Layout.fillWidth: true }
                Button {
                    Layout.preferredWidth: 100
                    Layout.fillHeight: true
                    text: "DASHCA"
                    font.pixelSize: 14
                    onClicked: root.activePage = 4;
                }
            }
        }
    }
}
