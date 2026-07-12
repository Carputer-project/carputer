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
    property int hoveredNavPage: -1
    property int focusedNavIndex: 0
    property string joypadLastCmd: ""
    property bool navBarFocused: false
    onActivePageChanged: {
        configManager.lastPage = activePage
        focusedNavIndex = activePage - 1
        navBarFocused = false
        Qt.callLater(function() {
            pageLoader.item && pageLoader.item.forceActiveFocus()
        })
    }
    Item {
        id: mainItem
        anchors.fill: parent
        activeFocusOnTab: false
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
                case 6:  return "RadioPage.qml"
                case 7:  return "TripLogPage.qml"
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
            height: 96
            color: Qt.rgba(themeManager.bgPanel.r, themeManager.bgPanel.g, themeManager.bgPanel.b, 0.95)
            border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.35)
            border.width: 1
            visible: root.menuVisible
            z: 100
            Rectangle {
                anchors { left: parent.left; right: parent.right; top: parent.top }
                height: 1
                color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.5)
            }
            Row {
                anchors.fill: parent
                spacing: 0
                Repeater {
                    id: navRepeater
                    model: [
                        { icon: "⬡", label: "DASH",    page: 1 },
                        { icon: "♫", label: "MEDIA",   page: 2 },
                        { icon: "◴", label: "RADIO",   page: 6 },
                        { icon: "◉", label: "REAR",    page: 3 },
                        { icon: "⏺", label: "DVR",     page: 4 },
                        { icon: "▦", label: "TRIP",    page: 7 },
                         { icon: "⚙", label: "SETUP",   page: 5 }
                    ]
                        Rectangle {
                        id: navItem
                        width: bottomBar.width / navRepeater.count
                        height: bottomBar.height
                        property bool highlighted: root.hoveredNavPage === modelData.page || root.activePage === modelData.page
                        color: root.activePage === modelData.page
                               ? Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.18)
                               : root.hoveredNavPage === modelData.page
                                 ? Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.10)
                                 : themeManager.transparent
                        Behavior on color { ColorAnimation { duration: 140 } }
                        // Top accent line for active tab
                        Rectangle {
                            anchors { top: parent.top; left: parent.left; right: parent.right }
                            height: 3
                            color: root.activePage === modelData.page ? themeManager.carBlue : themeManager.transparent
                        }
                        // Focus highlight ring (when nav-focused with arrow keys)
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 4
                            radius: 8
                            color: themeManager.transparent
                            border.width: root.focusedNavIndex === index ? 2 : 0
                            border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.6)
                            visible: root.menuVisible && root.focusedNavIndex === index
                        }
                        Column {
                            anchors.centerIn: parent
                            spacing: 4
                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: modelData.icon
                                color: root.activePage === modelData.page ? themeManager.carBlue : themeManager.textSecondary
                                font.pixelSize: 26
                                opacity: navItem.highlighted ? 1.0 : 0.85
                            }
                            Text {
                                anchors.horizontalCenter: parent.horizontalCenter
                                text: modelData.label
                                color: root.activePage === modelData.page ? themeManager.carBlue : themeManager.textSecondary
                                font.pixelSize: 12
                                font.bold: root.activePage === modelData.page
                                font.family: themeManager.fontFamily
                                opacity: navItem.highlighted ? 1.0 : 0.9
                            }
                        }
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: root.hoveredNavPage = modelData.page
                            onExited: root.hoveredNavPage = -1
                            onClicked: root.activePage = modelData.page
                        }
                    }
                }
            }
            // Joypad last command indicator (always visible in bottom bar)
            Text {
                anchors { right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: 16 }
                text: root.joypadLastCmd ? "CMD: " + root.joypadLastCmd : ""
                color: root.joypadLastCmd ? themeManager.carBlue : themeManager.transparent
                font.pixelSize: 14
                font.bold: true
                font.family: themeManager.fontFamily
                visible: root.joypadLastCmd.length > 0
            }
        }
        Connections {
            target: carControlManager
            function showJoypad(cmd) {
                root.joypadLastCmd = cmd
            }
            function onJoypadUp() {
                showJoypad("UP")
                if (root.navBarFocused) {
                    root.navBarFocused = false
                    if (pageLoader.item) pageLoader.item.forceActiveFocus()
                    return
                }
                if (pageLoader.item && pageLoader.item.navigateUp)
                    pageLoader.item.navigateUp()
            }
            function onJoypadDown() {
                showJoypad("DOWN")
                if (root.navBarFocused) return
                if (pageLoader.item && pageLoader.item.navigateDown) {
                    if (!pageLoader.item.navigateDown()) {
                        root.navBarFocused = true
                        root.focusedNavIndex = root.activePage - 1
                    }
                }
            }
            function onJoypadLeft() {
                showJoypad("LEFT")
                if (root.navBarFocused) {
                    root.focusedNavIndex = (root.focusedNavIndex + 5) % 6
                    return
                }
                if (pageLoader.item && pageLoader.item.navigateLeft)
                    pageLoader.item.navigateLeft()
            }
            function onJoypadRight() {
                showJoypad("RIGHT")
                if (root.navBarFocused) {
                    root.focusedNavIndex = (root.focusedNavIndex + 1) % 6
                    return
                }
                if (pageLoader.item && pageLoader.item.navigateRight)
                    pageLoader.item.navigateRight()
            }
            function onJoypadSelect() {
                showJoypad("SELECT")
                if (root.navBarFocused) {
                    root.activePage = root.focusedNavIndex + 1
                    root.navBarFocused = false
                    return
                }
                if (pageLoader.item && pageLoader.item.activateCurrentFocus)
                    pageLoader.item.activateCurrentFocus()
            }
            function onJoypadExit() {
                showJoypad("EXIT")
                if (root.navBarFocused) {
                    root.navBarFocused = false
                    return
                }
                if (pageLoader.item && pageLoader.item.handleEscape)
                    pageLoader.item.handleEscape()
            }
            function onVolumeSync(percent) {
                mediaManager.setVolume(percent)
            }
        }
        Keys.onPressed: {
            if (event.key === Qt.Key_Left) {
                root.joypadLastCmd = "LEFT";
                if (!root.menuVisible) root.menuVisible = true;
                var prevKb = root.focusedNavIndex - 1;
                if (prevKb < 0) prevKb = 5;
                root.focusedNavIndex = prevKb;
                event.accepted = true;
                return;
            }
            if (event.key === Qt.Key_Right) {
                root.joypadLastCmd = "RIGHT";
                if (!root.menuVisible) root.menuVisible = true;
                var nextKb = root.focusedNavIndex + 1;
                if (nextKb > 5) nextKb = 0;
                root.focusedNavIndex = nextKb;
                event.accepted = true;
                return;
            }
            if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                root.joypadLastCmd = "SELECT";
                if (root.menuVisible) {
                    root.activePage = root.focusedNavIndex + 1;
                } else {
                    Qt.callLater(function() {
                        pageLoader.item && pageLoader.item.forceActiveFocus()
                    });
                }
                event.accepted = true;
                return;
            }
            if (event.key === Qt.Key_Escape || event.key === Qt.Key_Back) {
                root.joypadLastCmd = "EXIT";
                root.activePage = 1;
                event.accepted = true;
                return;
            }
            // Number keys 1-8 for page navigation
            if (event.key >= Qt.Key_1 && event.key <= Qt.Key_8) {
                var pages = [1, 2, 3, 4, 5, 6, 7];
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
            // Media controls
            if (event.key === Qt.Key_Space) {
                if (mediaManager.playing)
                    mediaManager.pause();
                else
                    mediaManager.play();
                event.accepted = true;
                return;
            }
            if (event.key === Qt.Key_Left && (event.modifiers & Qt.ControlModifier)) {
                mediaManager.previous();
                event.accepted = true;
                return;
            }
            if (event.key === Qt.Key_Right && (event.modifiers & Qt.ControlModifier)) {
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
