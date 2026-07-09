import QtQuick 2.15
import QtQuick.Controls 2.15
Item {
    id: carplayPage
    focus: true
    property int focusIndex: -1
    Column {
        anchors.fill: parent
        padding: 10
        spacing: 10
        // Header
        Rectangle {
            width: parent.width - 20
            height: 50
            color: themeManager.bgCard
            radius: 8
            Row {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 15
                // CarPlay icon
                Text {
                    text: "📱"
                    font.pixelSize: 24
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: "APPLE CARPLAY"
                    color: themeManager.carBlue
                    font.pixelSize: 20
                    font.bold: true
                    anchors.verticalCenter: parent.verticalCenter
                }
                Item { width: 20; height: 1 }
                // Connection status
                Rectangle {
                    width: 12
                    height: 12
                    radius: 6
                    color: carPlayManager.connected ? themeManager.statusGreen : (carPlayManager.connectionStatus.indexOf("Connecting") >= 0 ? themeManager.statusYellow : themeManager.statusRed)
                    anchors.verticalCenter: parent.verticalCenter
                }
                Text {
                    text: carPlayManager.connectionStatus
                    color: themeManager.textSecondary
                    font.pixelSize: 14
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
        }
        // Main content area
        Rectangle {
            id: videoContainer
            width: parent.width - 20
            height: parent.height - 180
            color: themeManager.black
            radius: 8
            clip: true
            // CarPlay video surface
            Rectangle {
                id: videoSurface
                anchors.fill: parent
                anchors.margins: 2
                color: themeManager.bgDark
                visible: carPlayManager.streaming
                // Touch input handler
                MultiPointTouchArea {
                    anchors.fill: parent
                    minimumTouchPoints: 1
                    maximumTouchPoints: 5
                    onPressed: {
                        for (var i = 0; i < touchPoints.length; i++) {
                            var pt = touchPoints[i];
                            carPlayManager.sendTouchEvent(0, pt.x, pt.y, pt.pointId); // TouchDown
                        }
                    }
                    onUpdated: {
                        for (var i = 0; i < touchPoints.length; i++) {
                            var pt = touchPoints[i];
                            carPlayManager.sendTouchEvent(1, pt.x, pt.y, pt.pointId); // TouchMove
                        }
                    }
                    onReleased: {
                        for (var i = 0; i < touchPoints.length; i++) {
                            var pt = touchPoints[i];
                            carPlayManager.sendTouchEvent(2, pt.x, pt.y, pt.pointId); // TouchUp
                        }
                    }
                    onCanceled: {
                        for (var i = 0; i < touchPoints.length; i++) {
                            var pt = touchPoints[i];
                            carPlayManager.sendTouchEvent(3, pt.x, pt.y, pt.pointId); // TouchCancel
                        }
                    }
                }
                // Placeholder video frame (in production, use VideoOutput)
                Image {
                    id: videoFrame
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                    visible: carPlayManager.streaming
                    // source would be set from videoFrameImageReady signal
                }
                // Streaming indicator
                Text {
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    anchors.margins: 10
                    text: carPlayManager.videoWidth + "x" + carPlayManager.videoHeight + " @ " + carPlayManager.frameRate + "fps"
                    color: themeManager.textSecondary
                    font.pixelSize: 12
                    visible: carPlayManager.streaming
                }
            }
            // Connection prompt (shown when not connected)
            Column {
                anchors.centerIn: parent
                spacing: 20
                visible: !carPlayManager.streaming
                // CarPlay logo placeholder
                Rectangle {
                    width: 120
                    height: 120
                    radius: 60
                    color: themeManager.carBlueDim
                    anchors.horizontalCenter: parent.horizontalCenter
                    Text {
                        anchors.centerIn: parent
                        text: "▶"
                        font.pixelSize: 48
                        color: themeManager.carBlue
                    }
                }
                Text {
                    text: "Connect iPhone to Start CarPlay"
                    color: themeManager.textPrimary
                    font.pixelSize: 24
                    font.bold: true
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Text {
                    text: "Connect your iPhone via USB cable\nor enable Wireless CarPlay in iPhone settings"
                    color: themeManager.textSecondary
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Row {
                    spacing: 15
                    anchors.horizontalCenter: parent.horizontalCenter
                    Button {
                        width: 140
                        height: 50
                        text: carPlayManager.connected ? "Disconnect" : "Connect"
                        highlighted: carplayPage.focusIndex === 0
                        background: Rectangle {
                            color: carPlayManager.connected ? "#aa3333" : themeManager.carBlueDim
                            radius: 8
                        }
                        contentItem: Text {
                            text: parent.text
                            color: themeManager.textPrimary
                            font.pixelSize: 16
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: {
                            if (carPlayManager.connected) {
                                carPlayManager.stopConnection();
                            } else {
                                carPlayManager.startConnection();
                            }
                        }
                    }
                    Button {
                        width: 120
                        height: 50
                        text: "Scan"
                        highlighted: carplayPage.focusIndex === 1
                        visible: !carPlayManager.connected
                        background: Rectangle {
                            color: themeManager.bgCard
                            radius: 8
                            border.color: themeManager.carBlueDim
                            border.width: 1
                        }
                        contentItem: Text {
                            text: parent.text
                            color: themeManager.textPrimary
                            font.pixelSize: 16
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        onClicked: carPlayManager.scanForDevices()
                    }
                }
                // Device info when connected
                Text {
                    text: "📱 " + carPlayManager.deviceName
                    color: themeManager.carBlue
                    font.pixelSize: 16
                    visible: carPlayManager.deviceName.length > 0
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
        // Control bar
        Rectangle {
            width: parent.width - 20
            height: 70
            color: themeManager.bgCard
            radius: 8
            Row {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10
                // Siri button
                Button {
                    width: 60
                    height: 50
                    enabled: carPlayManager.connected
                    highlighted: carplayPage.focusIndex === 2
                    background: Rectangle {
                        color: parent.enabled ? themeManager.carBlueDim : themeManager.bgPanel
                        radius: 8
                    }
                    contentItem: Text {
                        text: "🎙"
                        font.pixelSize: 24
                        color: parent.enabled ? themeManager.textPrimary : themeManager.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: carPlayManager.sendSiriButton()
                }
                // Home button
                Button {
                    width: 60
                    height: 50
                    enabled: carPlayManager.connected
                    highlighted: carplayPage.focusIndex === 3
                    background: Rectangle {
                        color: parent.enabled ? themeManager.carBlueDim : themeManager.bgPanel
                        radius: 8
                    }
                    contentItem: Text {
                        text: "⌂"
                        font.pixelSize: 24
                        color: parent.enabled ? themeManager.textPrimary : themeManager.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: carPlayManager.sendHomeButton()
                }
                // Media controls
                Item { width: 20; height: 1 }
                Button {
                    width: 50
                    height: 50
                    enabled: carPlayManager.connected
                    highlighted: carplayPage.focusIndex === 4
                    background: Rectangle {
                        color: parent.enabled ? themeManager.bgPanel : themeManager.bgDark
                        radius: 8
                    }
                    contentItem: Text {
                        text: "⏮"
                        font.pixelSize: 20
                        color: parent.enabled ? themeManager.textPrimary : themeManager.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: carPlayManager.sendPreviousTrackButton()
                }
                Button {
                    width: 60
                    height: 50
                    enabled: carPlayManager.connected
                    highlighted: carplayPage.focusIndex === 5
                    background: Rectangle {
                        color: parent.enabled ? themeManager.bgPanel : themeManager.bgDark
                        radius: 8
                    }
                    contentItem: Text {
                        text: "⏯"
                        font.pixelSize: 24
                        color: parent.enabled ? themeManager.textPrimary : themeManager.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: carPlayManager.sendPlayPauseButton()
                }
                Button {
                    width: 50
                    height: 50
                    enabled: carPlayManager.connected
                    highlighted: carplayPage.focusIndex === 6
                    background: Rectangle {
                        color: parent.enabled ? themeManager.bgPanel : themeManager.bgDark
                        radius: 8
                    }
                    contentItem: Text {
                        text: "⏭"
                        font.pixelSize: 20
                        color: parent.enabled ? themeManager.textPrimary : themeManager.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: carPlayManager.sendNextTrackButton()
                }
                // Spacer
                Item { width: 20; height: 1 }
                // Volume slider
                Row {
                    spacing: 10
                    anchors.verticalCenter: parent.verticalCenter
                    Text {
                        text: "🔊"
                        font.pixelSize: 20
                        color: themeManager.textSecondary
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Slider {
                        width: 120
                        height: 30
                        from: 0
                        to: 100
                        value: carPlayManager.audioVolume
                        onMoved: carPlayManager.audioVolume = value
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }
                // Night mode toggle
                Button {
                    width: 50
                    height: 50
                    highlighted: carplayPage.focusIndex === 8
                    background: Rectangle {
                        color: carPlayManager.nightMode ? themeManager.carBlueDim : themeManager.bgPanel
                        radius: 8
                    }
                    contentItem: Text {
                        text: "🌙"
                        font.pixelSize: 20
                        color: themeManager.textPrimary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    onClicked: carPlayManager.nightMode = !carPlayManager.nightMode
                }
            }
        }
    }
    // Focus-based keyboard navigation
    function activateCurrentFocus() {
        if (focusIndex === 0) { carPlayManager.connected ? carPlayManager.stopConnection() : carPlayManager.startConnection(); return }
        if (focusIndex === 1) { carPlayManager.scanForDevices(); return }
        if (focusIndex === 2) { carPlayManager.sendSiriButton(); return }
        if (focusIndex === 3) { carPlayManager.sendHomeButton(); return }
        if (focusIndex === 4) { carPlayManager.sendPreviousTrackButton(); return }
        if (focusIndex === 5) { carPlayManager.sendPlayPauseButton(); return }
        if (focusIndex === 6) { carPlayManager.sendNextTrackButton(); return }
        if (focusIndex === 8) { carPlayManager.nightMode = !carPlayManager.nightMode; return }
    }

    function navigateLeft()  { if (focusIndex > 0) focusIndex-- }
    function navigateRight() { if (focusIndex < 8) focusIndex++ }
    function navigateDown()  { if (focusIndex < 2) { focusIndex = 2; return true } return false }
    function navigateUp()    { if (focusIndex > 1) { focusIndex = 0; return true } return true }

    Keys.onPressed: {
        if (event.key === Qt.Key_Left && focusIndex > 0) { focusIndex--; event.accepted = true; return }
        if (event.key === Qt.Key_Right && focusIndex < 8) { focusIndex++; event.accepted = true; return }
        if (event.key === Qt.Key_Down && focusIndex < 2) { focusIndex = 2; event.accepted = true; return }
        if (event.key === Qt.Key_Up && focusIndex > 1) { focusIndex = 0; event.accepted = true; return }
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            if (focusIndex === 0) { carPlayManager.connected ? carPlayManager.stopConnection() : carPlayManager.startConnection() }
            else if (focusIndex === 1) { carPlayManager.scanForDevices() }
            else if (focusIndex === 2) { carPlayManager.sendSiriButton() }
            else if (focusIndex === 3) { carPlayManager.sendHomeButton() }
            else if (focusIndex === 4) { carPlayManager.sendPreviousTrackButton() }
            else if (focusIndex === 5) { carPlayManager.sendPlayPauseButton() }
            else if (focusIndex === 6) { carPlayManager.sendNextTrackButton() }
            else if (focusIndex === 7) { } // volume slider
            else if (focusIndex === 8) { carPlayManager.nightMode = !carPlayManager.nightMode }
            event.accepted = true; return
        }
        if (event.key === Qt.Key_Space) {
            if (carPlayManager.connected) carPlayManager.sendPlayPauseButton()
            event.accepted = true; return
        }
    }
    // Auto-connect on page load
    Component.onCompleted: {
        if (!carPlayManager.connected) {
            carPlayManager.startConnection();
        }
    }
}
