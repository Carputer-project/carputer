import QtQuick 2.15
import QtQuick.Controls 2.15
Item {
    id: carplayPage
    focus: true
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
    // Keyboard shortcuts
    Keys.onPressed: {
        if (!carPlayManager.connected) return;
        switch (event.key) {
        case Qt.Key_Space:
            carPlayManager.sendPlayPauseButton();
            event.accepted = true;
            break;
        case Qt.Key_Left:
            carPlayManager.sendPreviousTrackButton();
            event.accepted = true;
            break;
        case Qt.Key_Right:
            carPlayManager.sendNextTrackButton();
            event.accepted = true;
            break;
        case Qt.Key_H:
            carPlayManager.sendHomeButton();
            event.accepted = true;
            break;
        case Qt.Key_S:
            carPlayManager.sendSiriButton();
            event.accepted = true;
            break;
        case Qt.Key_N:
            carPlayManager.nightMode = !carPlayManager.nightMode;
            event.accepted = true;
            break;
        }
    }
    // Auto-connect on page load
    Component.onCompleted: {
        if (!carPlayManager.connected) {
            carPlayManager.startConnection();
        }
    }
}
