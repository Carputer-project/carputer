import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
Item {
    id: mediaPage
    focus: true;
    property int browseMode: 0
    property bool showEq: false
    property int focusIndex: -1
    property var focusables: [
        "srcUsb", "srcSd", "srcInternal", "srcHome",
        "btnRepeat", "btnPrev", "btnPlay", "btnNext", "btnShuffle",
        "seekSlider", "volSlider", "btnEq",
        "tabPlaylist", "tabAlbums", "trackList"
    ]

    function activateItem(idx) {
        if (idx < 0 || idx >= focusables.length) return
        var name = focusables[idx]
        if (name === "srcUsb") mediaManager.scanMedia("/media/usb")
        else if (name === "srcSd") mediaManager.scanMedia("/media/sd")
        else if (name === "srcInternal") mediaManager.scanMedia("/root")
        else if (name === "srcHome") mediaManager.scanMedia("/home")
        else if (name === "btnRepeat") {
            var nextMode = (mediaManager.repeatMode + 1) % 3;
            mediaManager.setRepeatMode(nextMode);
        }
        else if (name === "btnPrev") mediaManager.previous()
        else if (name === "btnPlay") {
            if (mediaManager.playing) mediaManager.pause(); else mediaManager.play();
        }
        else if (name === "btnNext") mediaManager.next()
        else if (name === "btnShuffle") mediaManager.setShuffleOn(!mediaManager.shuffleOn);
        else if (name === "btnEq") showEq = !showEq
        else if (name === "tabPlaylist") browseMode = 0
        else if (name === "tabAlbums") browseMode = 1
    }

    function focusIdx(name) {
        var idx = focusables.indexOf(name)
        if (idx >= 0) focusIndex = idx
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_Left && focusIndex > 0) {
            focusIndex--
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_Right && focusIndex < focusables.length - 1) {
            focusIndex++
            event.accepted = true
            return
        }
        if (event.key === Qt.Key_Up) {
            if (focusIndex >= 9 && focusIndex <= 10) { focusIndex = 4; event.accepted = true; return }
            if (focusIndex >= 12) { focusIndex = 11; event.accepted = true; return }
            if (focusIndex === 11) { focusIndex = 9; event.accepted = true; return }
        }
        if (event.key === Qt.Key_Down) {
            if (focusIndex >= 0 && focusIndex <= 3) { focusIndex = 4; event.accepted = true; return }
            if (focusIndex >= 4 && focusIndex <= 8) { focusIndex = 9; event.accepted = true; return }
            if (focusIndex === 9 || focusIndex === 10) { focusIndex = 11; event.accepted = true; return }
            if (focusIndex === 11) { focusIndex = 12; event.accepted = true; return }
        }
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            if (focusIndex >= 0) activateItem(focusIndex)
            event.accepted = true
            return
        }
    }

    Rectangle {
        anchors.fill: parent;
        color: themeManager.bgDark;
        Column {
            anchors.fill: parent;
            anchors.margins: 10;
            spacing: 10;
            // ── Title bar ─────────────────────────────────────────
            Rectangle {
                width: parent.width;
                height: 60;
                color: themeManager.bgCard;
                radius: 8;
                border.width: 1
                border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.30)
                Rectangle {
                    anchors.fill: parent
                    radius: parent.radius
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.14) }
                        GradientStop { position: 0.5; color: "transparent" }
                    }
                }
                Text {
                    anchors.centerIn: parent;
                    text: "MEDIA PLAYER";
                    color: themeManager.carBlue;
                    font.pixelSize: 24;
                    font.bold: true;
                }
            }
            // ── Source buttons ──────────────────────────────────────────
            Row {
                spacing: 10;
                width: parent.width;
                Button {
                    id: srcUsb; width: 100; height: 50;
                    text: "USB";
                    onClicked: mediaManager.scanMedia("/media/usb");
                }
                Button {
                    id: srcSd; width: 100; height: 50;
                    text: "SD Card";
                    onClicked: mediaManager.scanMedia("/media/sd");
                }
                Button {
                    id: srcInternal; width: 100; height: 50;
                    text: "Internal";
                    onClicked: mediaManager.scanMedia("/root");
                }
                Button {
                    id: srcHome; width: 100; height: 50;
                    text: "Home";
                    onClicked: mediaManager.scanMedia("/home");
                }
            }
            // ── Now-playing / transport ───────────────────────────────────
            Rectangle {
                width: parent.width;
                height: 380;
                color: themeManager.bgCard;
                radius: 8;
                border.width: 1
                border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.22)
                Column {
                    anchors.fill: parent;
                    anchors.margins: 15;
                    spacing: 10;
                    Text {
                        text: "Now Playing";
                        color: themeManager.textSecondary;
                        font.pixelSize: 14;
                    }

                    // ── Artwork + track info row ──────────────────────────
                    Row {
                        width: parent.width
                        height: 80
                        spacing: 12

                        Rectangle {
                            width: 80
                            height: 80
                            radius: 6
                            color: themeManager.bgDark
                            border.width: 1
                            border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.25)
                            clip: true

                            Image {
                                id: artworkImage
                                anchors.fill: parent
                                source: mediaManager.artworkUrl.length > 0 ? mediaManager.artworkUrl : ""
                                fillMode: Image.PreserveAspectCrop
                                visible: status === Image.Ready
                                cache: false
                                asynchronous: true
                                sourceSize: Qt.size(160, 160)
                            }

                            Text {
                                anchors.centerIn: parent
                                text: "♫"
                                font.pixelSize: 32
                                color: themeManager.textSecondary
                                visible: artworkImage.status !== Image.Ready
                            }
                        }

                        Column {
                            width: parent.width - 92
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 4

                            Text {
                                text: mediaManager.currentTitle || mediaManager.currentTrack || "No track loaded"
                                color: themeManager.textPrimary
                                font.pixelSize: 16
                                font.bold: true
                                elide: Text.ElideRight
                                width: parent.width
                            }
                            Text {
                                text: mediaManager.currentArtist || ""
                                color: themeManager.carBlue
                                font.pixelSize: 13
                                elide: Text.ElideRight
                                width: parent.width
                                visible: text !== ""
                            }
                            Text {
                                text: mediaManager.currentAlbum || ""
                                color: themeManager.textSecondary
                                font.pixelSize: 12
                                elide: Text.ElideRight
                                width: parent.width
                                visible: text !== ""
                            }
                        }
                    }
                    // ── Spectrum Visualizer ───────────────────────────────
                    Rectangle {
                        width: parent.width
                        height: 70
                        color: themeManager.bgDark
                        radius: 6
                        border.width: 1
                        border.color: Qt.rgba(themeManager.carBlue.r, themeManager.carBlue.g, themeManager.carBlue.b, 0.18)
                        clip: true
                        Row {
                            anchors {
                                fill: parent
                                leftMargin: 8
                                rightMargin: 8
                                bottomMargin: 4
                                topMargin: 4
                            }
                            spacing: 2
                            Repeater {
                                model: mediaManager.spectrumData.length > 0
                                       ? mediaManager.spectrumData
                                       : Array(32).fill(-80.0)
                                Item {
                                    width: (parent.width - 31 * 2) / 32
                                    height: parent.height
                                    Rectangle {
                                        property real db: modelData
                                        property real normalized: Math.max(0.0, (db + 80.0) / 80.0)
                                        width: parent.width
                                        height: Math.max(3, normalized * parent.height)
                                        anchors.bottom: parent.bottom
                                        radius: 2
                                        color: normalized < 0.5
                                            ? Qt.rgba(0, 0.66 + normalized * 0.68, 0.91, 1.0)
                                            : Qt.rgba(0, 1.0, 0.91 - (normalized - 0.5) * 1.4, 1.0)
                                        Behavior on height {
                                            NumberAnimation { duration: 60; easing.type: Easing.OutQuart }
                                        }
                                        Behavior on color {
                                            ColorAnimation { duration: 60 }
                                        }
                                    }
                                }
                            }
                        }
                        Rectangle {
                            anchors.fill: parent
                            radius: 6
                            color: "transparent"
                            border.color: themeManager.bgCard
                            border.width: 1
                        }
                    }
                    // ── Transport + Repeat/Shuffle ──────────────────────────
                    Row {
                        spacing: 10;
                        anchors.horizontalCenter: parent.horizontalCenter;

                        Rectangle {
                            id: btnRepeat
                            width: 40; height: 40; radius: 4;
                            color: mediaManager.repeatMode > 0 ? themeManager.carBlue : themeManager.bgDark;
                            border.color: focusIndex === 4 ? themeManager.carBlue : themeManager.textSecondary;
                            border.width: focusIndex === 4 ? 2 : 1;
                            Text {
                                anchors.centerIn: parent;
                                text: mediaManager.repeatMode === 2 ? "1" : "↻";
                                color: mediaManager.repeatMode > 0 ? "white" : themeManager.textSecondary;
                                font.pixelSize: 16;
                                font.bold: true;
                            }
                            MouseArea {
                                anchors.fill: parent;
                                onClicked: { mediaPage.focusIdx("btnRepeat"); activateItem(4); }
                            }
                        }

                        Button {
                            id: btnPrev; width: 50; height: 40;
                            text: "PREV";
                            font.pixelSize: 13;
                            font.bold: true;
                            onClicked: { mediaPage.focusIdx("btnPrev"); mediaManager.previous(); }
                        }
                        Button {
                            id: btnPlay; width: 70; height: 50;
                            text: mediaManager.playing ? "PAUSE" : "PLAY";
                            font.pixelSize: 14;
                            font.bold: true;
                            onClicked: {
                                mediaPage.focusIdx("btnPlay");
                                if (mediaManager.playing) mediaManager.pause(); else mediaManager.play();
                            }
                        }
                        Button {
                            id: btnNext; width: 50; height: 40;
                            text: "NEXT";
                            font.pixelSize: 13;
                            font.bold: true;
                            onClicked: { mediaPage.focusIdx("btnNext"); mediaManager.next(); }
                        }

                        Rectangle {
                            id: btnShuffle
                            width: 40; height: 40; radius: 4;
                            color: mediaManager.shuffleOn ? themeManager.carBlue : themeManager.bgDark;
                            border.color: focusIndex === 8 ? themeManager.carBlue : themeManager.textSecondary;
                            border.width: focusIndex === 8 ? 2 : 1;
                            Text {
                                anchors.centerIn: parent;
                                text: "⇄";
                                color: mediaManager.shuffleOn ? "white" : themeManager.textSecondary;
                                font.pixelSize: 16;
                                font.bold: true;
                            }
                            MouseArea {
                                anchors.fill: parent;
                                onClicked: { mediaPage.focusIdx("btnShuffle"); mediaManager.setShuffleOn(!mediaManager.shuffleOn); }
                            }
                        }
                    }
                    // Seek slider
                    Slider {
                        id: seekSlider
                        width: parent.width;
                        from: 0;
                        to: mediaManager.duration || 100;
                        value: mediaManager.position;
                        onMoved: mediaManager.seek(Math.round(value));
                        onPressedChanged: if (pressed) mediaPage.focusIdx("seekSlider")
                    }
                    Text {
                        text: formatTime(mediaManager.position) + " / " + formatTime(mediaManager.duration);
                        color: themeManager.textSecondary;
                        font.pixelSize: 12;
                        anchors.horizontalCenter: parent.horizontalCenter;
                    }
                    // Volume row
                    Row {
                        spacing: 10;
                        anchors.horizontalCenter: parent.horizontalCenter;
                        Text {
                            text: "🔊";
                            color: themeManager.textPrimary;
                            anchors.verticalCenter: parent.verticalCenter;
                        }
                        Slider {
                            id: volSlider
                            width: 120;
                            from: 0;
                            to: 100;
                            value: mediaManager.volume;
                            onMoved: mediaManager.setVolume(Math.round(value));
                            onPressedChanged: if (pressed) mediaPage.focusIdx("volSlider")
                        }
                        Text {
                            text: mediaManager.volume + "%";
                            color: themeManager.textSecondary;
                            font.pixelSize: 12;
                            anchors.verticalCenter: parent.verticalCenter;
                        }
                    }
                    // EQ toggle
                    Row {
                        anchors.horizontalCenter: parent.horizontalCenter
                        spacing: 10
                        Button {
                            id: btnEq
                            text: showEq ? "EQ ▲" : "EQ ▼"
                            width: 60; height: 30
                            font.pixelSize: 12
                            onClicked: showEq = !showEq
                        }
                    }
                    // EQ section
                    Column {
                        visible: showEq
                        spacing: 2
                        width: parent.width
                        // Bass - 100 Hz
                        Row {
                            spacing: 6; width: parent.width
                            Text { text: "Bass"; width: 42; font.pixelSize: 12; color: themeManager.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                            Text { text: "100Hz"; width: 42; font.pixelSize: 10; color: themeManager.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                            Slider {
                                width: parent.width - 170; from: -24; to: 12; stepSize: 0.5
                                value: mediaManager.eqBand0
                                onMoved: mediaManager.setEqBand(0, value)
                            }
                            Text {
                                text: mediaManager.eqBand0.toFixed(1) + "dB"
                                width: 50; font.pixelSize: 11; color: themeManager.textSecondary
                                anchors.verticalCenter: parent.verticalCenter; horizontalAlignment: Text.AlignRight
                            }
                        }
                        // Mid - 1100 Hz
                        Row {
                            spacing: 6; width: parent.width
                            Text { text: "Mid"; width: 42; font.pixelSize: 12; color: themeManager.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                            Text { text: "1100Hz"; width: 42; font.pixelSize: 10; color: themeManager.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                            Slider {
                                width: parent.width - 170; from: -24; to: 12; stepSize: 0.5
                                value: mediaManager.eqBand1
                                onMoved: mediaManager.setEqBand(1, value)
                            }
                            Text {
                                text: mediaManager.eqBand1.toFixed(1) + "dB"
                                width: 50; font.pixelSize: 11; color: themeManager.textSecondary
                                anchors.verticalCenter: parent.verticalCenter; horizontalAlignment: Text.AlignRight
                            }
                        }
                        // Treble - 11 kHz
                        Row {
                            spacing: 6; width: parent.width
                            Text { text: "Treble"; width: 42; font.pixelSize: 12; color: themeManager.textPrimary; anchors.verticalCenter: parent.verticalCenter }
                            Text { text: "11kHz"; width: 42; font.pixelSize: 10; color: themeManager.textSecondary; anchors.verticalCenter: parent.verticalCenter }
                            Slider {
                                width: parent.width - 170; from: -24; to: 12; stepSize: 0.5
                                value: mediaManager.eqBand2
                                onMoved: mediaManager.setEqBand(2, value)
                            }
                            Text {
                                text: mediaManager.eqBand2.toFixed(1) + "dB"
                                width: 50; font.pixelSize: 11; color: themeManager.textSecondary
                                anchors.verticalCenter: parent.verticalCenter; horizontalAlignment: Text.AlignRight
                            }
                        }
                        Button {
                            text: "RESET"
                            width: 70; height: 24
                            font.pixelSize: 11
                            anchors.horizontalCenter: parent.horizontalCenter
                            onClicked: mediaManager.resetEq()
                        }
                    }
                }
            }
            // ── Browse tabs ───────────────────────────────────────────
            Row {
                spacing: 5;
                width: parent.width;
                Repeater {
                    model: ["Playlist", "Albums"]
                    Rectangle {
                        width: (parent.width - 5) / 2;
                        height: 32;
                        radius: 4;
                        color: browseMode === index ? themeManager.carBlue : themeManager.bgCard;
                        border.width: (mediaPage.focusIndex === 11 + index) ? 2 : 0;
                        border.color: themeManager.carBlue;
                        Text {
                            anchors.centerIn: parent;
                            text: modelData;
                            color: browseMode === index ? "white" : themeManager.textSecondary;
                            font.pixelSize: 14;
                            font.bold: browseMode === index;
                        }
                        MouseArea {
                            anchors.fill: parent;
                            onClicked: { mediaPage.focusIdx(mediaPage.focusables[11 + index]); browseMode = index; }
                        }
                    }
                }
            }
            // ── Playlist / Album list ──────────────────────────
            ListView {
                id: trackList
                width: parent.width;
                height: 200;
                model: browseMode === 0
                       ? mediaManager.playlistTracks
                       : mediaManager.playlistTracks;
                clip: true;
                currentIndex: mediaManager.currentIndex;
                property bool groupByAlbum: browseMode === 1;

                section.property: browseMode === 1 ? "dirName" : "";
                section.labelPositioning: ViewSection.InlineLabels;
                section.delegate: Rectangle {
                    width: ListView.view.width;
                    height: section === "" ? 0 : 28;
                    color: themeManager.bgCard;
                    radius: 3;
                    visible: section !== "";
                    Text {
                        anchors { left: parent.left; leftMargin: 8; verticalCenter: parent.verticalCenter; }
                        text: section;
                        color: themeManager.carBlue;
                        font.pixelSize: 13;
                        font.bold: true;
                        elide: Text.ElideRight;
                        width: parent.width - 16;
                    }
                }

                delegate: Rectangle {
                    width: ListView.view.width;
                    height: 36;
                    color: ListView.isCurrentItem ? themeManager.carBlueDim : "transparent";
                    radius: 3;
                    Row {
                        anchors { fill: parent; leftMargin: 12 + (browseMode === 1 ? 8 : 0); rightMargin: 8; }
                        spacing: 8;
                        Text {
                            text: browseMode === 0 ? (index + 1) : "";
                            color: themeManager.textSecondary;
                            font.pixelSize: 12;
                            width: browseMode === 0 ? 24 : 0;
                            anchors.verticalCenter: parent.verticalCenter;
                        }
                        Text {
                            text: modelData ? (modelData.fileName || "") : ""
                            color: themeManager.textPrimary;
                            font.pixelSize: 12;
                            width: parent.width - (browseMode === 0 ? 32 : 8);
                            elide: Text.ElideRight;
                            anchors.verticalCenter: parent.verticalCenter;
                        }
                    }
                    MouseArea {
                        anchors.fill: parent;
                        onClicked: { mediaPage.focusIdx("trackList"); mediaManager.playTrack(index); }
                    }
                }

                highlight: Rectangle {
                    color: themeManager.carBlue;
                    radius: 3;
                    opacity: 0.2;
                }
                highlightFollowsCurrentItem: true;

                Text {
                    anchors.centerIn: parent;
                    text: "No tracks loaded";
                    color: themeManager.textSecondary;
                    font.pixelSize: 14;
                    visible: mediaManager.playlistTracks.length === 0;
                }
            }
        }
    }
    function activateCurrentFocus() {
        if (focusIndex >= 0) activateItem(focusIndex)
    }

    function navigateLeft()  { if (focusIndex > 0) focusIndex-- }
    function navigateRight() { if (focusIndex < focusables.length - 1) focusIndex++ }
    function navigateUp() {
        if (focusIndex >= 9 && focusIndex <= 10) { focusIndex = 4; return true }
        if (focusIndex >= 12) { focusIndex = 11; return true }
        if (focusIndex === 11) { focusIndex = 9; return true }
        return true
    }
    function navigateDown() {
        if (focusIndex >= 0 && focusIndex <= 3) { focusIndex = 4; return true }
        if (focusIndex >= 4 && focusIndex <= 8) { focusIndex = 9; return true }
        if (focusIndex === 9 || focusIndex === 10) { focusIndex = 11; return true }
        if (focusIndex === 11) { focusIndex = 12; return true }
        return false
    }

    function formatTime(ms) {
        if (!ms || ms <= 0) return "0:00";
        var seconds = Math.floor(ms / 1000);
        var minutes = Math.floor(seconds / 60);
        seconds = seconds % 60;
        return minutes + ":" + (seconds < 10 ? "0" : "") + seconds;
    }
}
