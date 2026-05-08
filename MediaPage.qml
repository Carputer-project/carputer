import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
Item {
    focus: true;
    property int browseMode: 0 // 0=Playlist, 1=Albums

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
                    width: 100; height: 50;
                    text: "USB";
                    onClicked: mediaManager.scanMedia("/media/usb");
                }
                Button {
                    width: 100; height: 50;
                    text: "SD Card";
                    onClicked: mediaManager.scanMedia("/media/sd");
                }
                Button {
                    width: 100; height: 50;
                    text: "Internal";
                    onClicked: mediaManager.scanMedia("/root");
                }
                Button {
                    width: 100; height: 50;
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

                        // Album artwork
                        Rectangle {
                            width: 80
                            height: 80
                            radius: 6
                            color: themeManager.bgDark
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

                            // Placeholder when no artwork
                            Text {
                                anchors.centerIn: parent
                                text: "♫"
                                font.pixelSize: 32
                                color: themeManager.textSecondary
                                visible: artworkImage.status !== Image.Ready
                            }
                        }

                        // Track info column
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

                        // Repeat button
                        Rectangle {
                            width: 40; height: 40; radius: 4;
                            color: mediaManager.repeatMode > 0 ? themeManager.carBlue : themeManager.bgDark;
                            border.color: themeManager.textSecondary;
                            border.width: 1;
                            Text {
                                anchors.centerIn: parent;
                                text: mediaManager.repeatMode === 2 ? "1" : "↻";
                                color: mediaManager.repeatMode > 0 ? "white" : themeManager.textSecondary;
                                font.pixelSize: 16;
                                font.bold: true;
                            }
                            MouseArea {
                                anchors.fill: parent;
                                onClicked: {
                                    var nextMode = (mediaManager.repeatMode + 1) % 3;
                                    mediaManager.setRepeatMode(nextMode);
                                }
                            }
                        }

                        Button {
                            width: 50; height: 40;
                            text: "PREV";
                            font.pixelSize: 13;
                            font.bold: true;
                            onClicked: mediaManager.previous();
                        }
                        Button {
                            width: 70; height: 50;
                            text: mediaManager.playing ? "PAUSE" : "PLAY";
                            font.pixelSize: 14;
                            font.bold: true;
                            onClicked: {
                                if (mediaManager.playing)
                                    mediaManager.pause();
                                else
                                    mediaManager.play();
                            }
                        }
                        Button {
                            width: 50; height: 40;
                            text: "NEXT";
                            font.pixelSize: 13;
                            font.bold: true;
                            onClicked: mediaManager.next();
                        }

                        // Shuffle button
                        Rectangle {
                            width: 40; height: 40; radius: 4;
                            color: mediaManager.shuffleOn ? themeManager.carBlue : themeManager.bgDark;
                            border.color: themeManager.textSecondary;
                            border.width: 1;
                            Text {
                                anchors.centerIn: parent;
                                text: "⇄";
                                color: mediaManager.shuffleOn ? "white" : themeManager.textSecondary;
                                font.pixelSize: 16;
                                font.bold: true;
                            }
                            MouseArea {
                                anchors.fill: parent;
                                onClicked: mediaManager.setShuffleOn(!mediaManager.shuffleOn);
                            }
                        }
                    }
                    // Seek slider
                    Slider {
                        width: parent.width;
                        from: 0;
                        to: mediaManager.duration || 100;
                        value: mediaManager.position;
                        onMoved: mediaManager.seek(Math.round(value));
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
                            width: 120;
                            from: 0;
                            to: 100;
                            value: mediaManager.volume;
                            onMoved: mediaManager.setVolume(Math.round(value));
                        }
                        Text {
                            text: mediaManager.volume + "%";
                            color: themeManager.textSecondary;
                            font.pixelSize: 12;
                            anchors.verticalCenter: parent.verticalCenter;
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
                        Text {
                            anchors.centerIn: parent;
                            text: modelData;
                            color: browseMode === index ? "white" : themeManager.textSecondary;
                            font.pixelSize: 14;
                            font.bold: browseMode === index;
                        }
                        MouseArea {
                            anchors.fill: parent;
                            onClicked: browseMode = index;
                        }
                    }
                }
            }
            // ── Playlist / Album list ──────────────────────────
            ListView {
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
                        onClicked: mediaManager.playTrack(index);
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
    function formatTime(ms) {
        if (!ms || ms <= 0) return "0:00";
        var seconds = Math.floor(ms / 1000);
        var minutes = Math.floor(seconds / 60);
        seconds = seconds % 60;
        return minutes + ":" + (seconds < 10 ? "0" : "") + seconds;
    }
}
