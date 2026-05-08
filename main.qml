import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.impl 2.15

Window {
    id: root
    width: screenWidth
    height: screenHeight
    title: "Carputer"
    color: activePage === 1 ? "transparent" : themeManager.bgDark

    Component.onCompleted: {
        showFullScreen()
    }
    property int activePage: 1
    property bool menuVisible: true
    onActivePageChanged: {
        configManager.lastPage = activePage
        Qt.callLater(function() {
            pageLoader.item && pageLoader.item.forceActiveFocus()
        })
    }
    Item {
        id: mainItem
        anchors.fill: parent
        activeFocusOnTab: false
        focus: true
        Loader {
            id: pageLoader
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                bottom: root.menuVisible ? bottomBar.top : parent.bottom
            }
            source: {
                switch(root.activePage) {
                case 1:  return "DashboardPage.qml"
                case 2:  return "MediaPage.qml"
                case 3:  return "BackupCamPage.qml"
                case 4:  return "DashcamPage.qml"
                case 5:  return "SettingsPage.qml"
                default: return "DashboardPage.qml"
                }
            }
            onLoaded: {
                item.focus = true
                item.forceActiveFocus()
            }
        }
        Rectangle {
            id: bottomBar
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
            height: 90
            color: themeManager.bgPanel
            border.color: themeManager.carBlueDim
            border.width: 1
            visible: root.menuVisible
            z: 100
            Row {
                anchors.fill: parent
                spacing: 0
                Repeater {
                    id: navRepeater
                    model: [
                        { icon: "⬡", label: "DASH",    page: 1 },
                        { icon: "♫", label: "MEDIA",   page: 2 },
                        { icon: "◉", label: "REAR",    page: 3 },
                        { icon: "⏺", label: "DVR",     page: 4 },
                         { icon: "⚙", label: "SETUP",   page: 5 }
                    ]
                        Rectangle {
                        width: bottomBar.width / navRepeater.count
                        height: bottomBar.height
                        color: root.activePage === modelData.page ? themeManager.carBlueDim : themeManager.transparent
                        // Top accent line for active tab
                        Rectangle {
                            anchors { top: parent.top; left: parent.left; right: parent.right }
                            height: 3
                            color: root.activePage === modelData.page ? themeManager.carBlue : themeManager.transparent
                        }
                        Column {
                            anchors.centerIn: parent
                            spacing: 4
                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: modelData.icon
                                color: root.activePage === modelData.page ? themeManager.carBlue : themeManager.textSecondary
                                font.pixelSize: 26
                            }
                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: modelData.label
                                color: root.activePage === modelData.page ? themeManager.carBlue : themeManager.textSecondary
                                font.pixelSize: 12
                                font.bold: root.activePage === modelData.page
                                font.family: themeManager.fontFamily
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: parent.color = themeManager.carBlueDim
                            onExited: parent.color = (root.activePage === modelData.page ? themeManager.carBlueDim : themeManager.transparent)
                            onClicked: root.activePage = modelData.page
                        }
                    }
                }
            }
        }
        Keys.onPressed: {
            // Number keys 1-8 for page navigation
            if (event.key >= Qt.Key_1 && event.key <= Qt.Key_8) {
                var pages = [1, 2, 3, 4, 5];
                var newPage = pages[(event.key - Qt.Key_1)];
                if (newPage && newPage !== root.activePage) {
                    root.activePage = newPage;
                }
                event.accepted = true;
                return;
            }
            // M = Toggle menu
            if (event.key === Qt.Key_M) {
                root.menuVisible = !root.menuVisible;
                event.accepted = true;
                return;
            }
            // Escape = Show menu
            if (event.key === Qt.Key_Escape) {
                if (!root.menuVisible) {
                    root.menuVisible = true;
                }
                event.accepted = true;
                return;
            }
            // Media controls
            if (event.key === Qt.Key_Space) {
                if (mediaManager.playing)
                    mediaManager.pause();
                else
                    mediaManager.play();
                event.accepted = true;
                return;
            }
            if (event.key === Qt.Key_Left) {
                mediaManager.previous();
                event.accepted = true;
                return;
            }
            if (event.key === Qt.Key_Right) {
                mediaManager.next();
                event.accepted = true;
                return;
            }
            // B = Backup camera
            if (event.key === Qt.Key_B) {
                root.activePage = 3;
                event.accepted = true;
                return;
            }
            // R = DVR/Record
            if (event.key === Qt.Key_R) {
                if (dvrManager.recording) {
                    dvrManager.stopRecording();
                } else {
                    dvrManager.startRecording();
                }
                event.accepted = true;
                return;
            }
            // V = Media page
            if (event.key === Qt.Key_V) {
                root.activePage = 2;
                event.accepted = true;
                return;
            }
            // H = Home/Dashboard
            if (event.key === Qt.Key_H) {
                root.activePage = 1;
                event.accepted = true;
                return;
            }
            // N = Next track
            if (event.key === Qt.Key_N) {
                mediaManager.next();
                event.accepted = true;
                return;
            }
            // P = Previous track
            if (event.key === Qt.Key_P) {
                mediaManager.previous();
                event.accepted = true;
                return;
            }
        }
    }
}